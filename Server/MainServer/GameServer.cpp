#include "pch.h"
#include "../Define.h"
#include "ServerDefine.h"

#include "Zone.h"

#include "MemoryUnit.h"

#include "TimerManager.h"

#include "SendMemoryPool.h"

#include "UserData.h"

#include "ObjectInfo.h"

#include "GameServer.h"

GameServer::GameServer(bool inNotUse)
	: wsa()
	, hIOCP()
	, listenSocket()
	, serverAddr()
	, workerThreadCont()
	, zone(std::make_unique<Zone>())
{
	ServerIntegrityCheck();
	
	ERROR_HANDLING::errorRecvOrSendArr[0] = ERROR_HANDLING::HandleRecvOrSendError;
	ERROR_HANDLING::errorRecvOrSendArr[1] = ERROR_HANDLING::NotError;

	SendMemoryPool::MakeInstance();
	InitNetwork();

	TimerManager::MakeInstance(hIOCP);

	PrintServerInfoUI();
};

GameServer::~GameServer()
{
	SendMemoryPool::DeleteInstance();
	TimerManager::DeleteInstance();

	workerThreadCont.clear();

	closesocket(listenSocket);
	CloseHandle(hIOCP);
}

void GameServer::ServerIntegrityCheck()
{
	//무결성 검사
	static_assert(GLOBAL_DEFINE::MAX_HEIGHT == GLOBAL_DEFINE::MAX_WIDTH,
		"MAX_HEIGHT와 MAX_WIDTH가 다르며, 이는 현재로직에서 Sector 계산에서 비정상적인 결과를 도출할 수 있습니다. 서버 실행을 거절하였습니다.");

	static_assert((int)((GLOBAL_DEFINE::MAX_HEIGHT - 1) / GLOBAL_DEFINE::SECTOR_DISTANCE)
		!= (int)((GLOBAL_DEFINE::MAX_HEIGHT + 1) / GLOBAL_DEFINE::SECTOR_DISTANCE),
		"MAX_HEIGHT(그리고 MAX_WIDTH)는 SECTOR_DISTANCE의 배수가 아닐 경우, 비정상적인 결과를 도출할 수 있습니다. 서버 실행을 거절하였습니다.");
	
	// 이제 채팅서버에서 해당 내용을 검사합니다!
	//static_assert(PACKET_TYPE::CLIENT_TO_SERVER::CHAT == PACKET_TYPE::SERVER_TO_CLIENT::CHAT,
	//	"CS::CHAT와 SC::CHAT의 값이 다르며, 이는 클라이언트에 치명적인 오류를 발생시킵니다. 서버 실행을 거절하였습니다.");
}

/*
	GameServer::PrintServerInfoUI()
		- GamsServer의 생성자에서 호출되며, 서버의 UI들을 출력합니다.
*/
void GameServer::PrintServerInfoUI()
{
	printf("\n■■■■■■■■■■■■■■■■■■■■■■■■■\n");
	printf("■ 게임서버프로그래밍 숙제 5번   \n");
	printf("■                   게임공학과 원성연 2013182027\n");
	printf("■\n");
	
	// 추후 퍼블릭 IP로 변경.
	printf("■ IP : LocalHost(127.0.0.1)\n");
	printf("■ Listen Port : 9000\n");
	printf("■■■■■■■■■■■■■■■■■■■■■■■■■\n");
}

/*
	GameServer::InitNetwork()
		- GamsServer의 생성자에서 호출되며, 네트워크 통신과 관련된 초기화를 담당합니다.
*/
void GameServer::InitNetwork()
{
	using namespace ERROR_HANDLING;

	// 1. 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) ERROR_QUIT(TEXT("WSAStartup()"));

	// 2. 입출력 완료 포트 생성
	if (hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)
		; hIOCP == NULL) ERROR_QUIT(L"Create_IOCompletionPort()");

	// 현재는 CPU 개수 확인할 필요 없음.
	//SYSTEM_INFO si;
	//GetSystemInfo(&si);

	// 3. 워커 쓰레드 생성 및 IOCP 등록.
	workerThreadCont.reserve(4);
	printf("!. 현재 워커쓰레드 개수는 코어의 개수와 상관없이 4개로 제한, 생성합니다. \n");
	for (int i = 0; i < /* (int)si.dwNumberOfProcessors * 2 */ 4; ++i)
	{
		workerThreadCont.emplace_back(std::thread{ StartWorkerThread, (LPVOID)this });
	}

	// 4. 소켓 생성
	if (listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)
		; listenSocket == INVALID_SOCKET) ERROR_QUIT(TEXT("socket()"));

	// 5. 서버 정보 객체 설정
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(GLOBAL_DEFINE::MAIN_SERVER_PORT);

	// 6. 소켓 설정
	if (::bind(listenSocket, (SOCKADDR *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) ERROR_QUIT(TEXT("bind()"));

	// 7. Listen()!
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) ERROR_QUIT(TEXT("listen()"));
}

/*
	GameServer::Run()
		- Accept Process 실행 및 Worker Thread join!
*/
void GameServer::Run()
{
	std::thread timerThread{ TimerManager::GetInstance()->StartTimerThread };
	
	// TimerManaer가 먼저! 여야! 해!
	std::thread acceptThread{ StartAcceptThread, (LPVOID)this };

	printf("\n\n#. Game Server activated!\n\n");
	
	timerThread.join();
	acceptThread.join();
	for (auto& thread : workerThreadCont) thread.join();
}

/*
	GameServer::StartAcceptThread(LPVOID arg)
		- 쓰레드에서 멤버 변수를 사용하기 위해, 클래스 내부에서 엑셉트 쓰레드에 필요한 함수를 호출.
*/
void GameServer::StartAcceptThread(LPVOID arg)
{
	GameServer* pServer = static_cast<GameServer*>(arg);
	pServer->AcceptThreadFunction();
};

/*
	GameServer::AcceptThreadFunction()
		- Accept를 담당합니다.

		#0. InNewClient에서 SocketInfo 할당이 이루어집니다.
		#1. InNewClient의 first가 false일 경우는, 동접보다 많은 수의 플레이어가 접속하려할 때 입니다. 
*/
void GameServer::AcceptThreadFunction()
{
	SOCKET clientSocket{};
	SOCKADDR_IN clientAddr{};
	int addrLength = sizeof(clientAddr);

	while (7) {
		//accept()
		if (clientSocket = WSAAccept(listenSocket, (SOCKADDR *)&clientAddr, &addrLength, NULL, NULL)
			; clientSocket == INVALID_SOCKET)
		{
			ERROR_HANDLING::ERROR_QUIT(TEXT("accept()"));
			break;
		}

		if (auto [isTrueAdd, pClient] = zone->TryToEnter()
			; isTrueAdd)
		{
			// 소켓과 입출력 완료 포트 연결
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), hIOCP, pClient->objectInfo->key, 0);

			pClient->sock = clientSocket;
			
			// MemoryUnit 생성자에서 보장함.
			//pClient->memoryUnit.wsaBuf.buf = pClient->memoryUnit.dataBuf;
			//pClient->memoryUnit.wsaBuf.len = GLOBAL_DEFINE::MAX_SIZE_OF_RECV;

			// 클라이언트에게 서버에 접속(Accept) 함을 알림
			PACKET_DATA::MAIN_TO_CLIENT::LoginOk loginPacket(pClient->objectInfo->key);
			NETWORK_UTIL::SendPacket(pClient, reinterpret_cast<char*>(&loginPacket));

			// 자신의 캐릭터를 넣어줌.
			PACKET_DATA::MAIN_TO_CLIENT::PutPlayer putPacket( pClient->objectInfo->key, pClient->objectInfo->posX, pClient->objectInfo->posY);
			NETWORK_UTIL::SendPacket(pClient, reinterpret_cast<char*>(&putPacket));

			std::cout << " [HELLO] 클라이언트 (" << inet_ntoa(clientAddr.sin_addr) << ") 가 접속했습니다. \n";
			
			// 최초 위치에서 처음 뷰리스트와 섹터 갱신.
			pClient->pZone->InitViewAndSector(pClient);

			// 비동기 입출력의 시작.
			NETWORK_UTIL::RecvPacket(pClient);
		}
		else {
			closesocket(clientSocket);
			//delete pClient;	// if nullptr;
		}
	}
}

/*
	GameServer::StartWorkerThread(LPVOID arg)
		- 쓰레드에서 멤버 변수를 사용하기 위해, 클래스 내부에서 워커쓰레드에 필요한 함수를 호출.
*/
void GameServer::StartWorkerThread(LPVOID arg)
{
	GameServer* pServer = static_cast<GameServer*>(arg);
	pServer->WorkerThreadFunction();
};

/*
	GameServer::WorkerThreadFunction()
		- 워커 쓰레드 함수.
*/
void GameServer::WorkerThreadFunction()
{
	// 한 번만 선언해서 여러번 씁시다. 아껴써야지...
	int retVal{};
	DWORD cbTransferred;
	unsigned long long Key;
	
	LPVOID pMemoryUnit{nullptr};

	while (7)
	{
#pragma region [ Wait For Event ]
		// 입출력 완료 포트에 저장된 결과를 처리하기 위한 함수 // 대기 상태가 됨
		retVal = GetQueuedCompletionStatus(
			hIOCP, //입출력 완료 포트 핸들
			&cbTransferred, //비동기 입출력 작업으로, 전송된 바이트 수가 여기에 저장된다.
			&Key, //함수 호출 시 전달한 세번째 인자(32비트) 가 여기에 저장된다.
			reinterpret_cast<LPOVERLAPPED*>(&pMemoryUnit /*pReturnPointer*/), //Overlapped 구조체의 주소값
			INFINITE // 대기 시간 -> 깨울 까지 무한대
		);
#pragma endregion

		//pMemoryUnit = pReturnPointer - sizeof(MEMORY_UNIT_TYPE);
		//std::cout << "출력값 : " << (int&)(*(&pReturnPointer - 4)) << std::endl;

//#pragma region [ Error Exception ]
//		//ERROR_CLIENT_DISCONNECT:
//		if (retVal == 0 || cbTransferred == 0)
//		{
//			NETWORK_UTIL::LogOutProcess(pMemoryUnit);
//			continue;
//		}
//#pragma endregion

#pragma region [ Process ]
		if (MemoryUnit * pTemp = reinterpret_cast<MemoryUnit*>(pMemoryUnit)
			; MEMORY_UNIT_TYPE::SEND_TO_CLIENT == pTemp->memoryUnitType)
		{
			// std::cout << "\n[SEND_TO_CLIENT] \n";

			// send에 대한 에러는 처리하지 않음.
			//if (retVal == 0 || cbTransferred == 0)
			//{
			//	NETWORK_UTIL::LogOutProcess(pMemoryUnit);
			//	continue;
			//}

			AfterSend(reinterpret_cast<SendMemoryUnit*>(pMemoryUnit));
		}
		else if (MEMORY_UNIT_TYPE::TIMER_FUNCTION == pTemp->memoryUnitType)
		{
			//std::cout << "\n[TIMER_FUNCTION] \n";
			ProcessTimerUnit(Key);
		}	
		else if (MEMORY_UNIT_TYPE::RECV_FROM_CLIENT == pTemp->memoryUnitType)
		{
			//std::cout << "\n[RECV_FROM_CLIENT] \n";
			if (retVal == 0 || cbTransferred == 0)
			{
				NETWORK_UTIL::LogOutProcess(pMemoryUnit);
				continue;
			}
			AfterRecv(reinterpret_cast<SocketInfo*>(pMemoryUnit), cbTransferred);
		}
		else
		{
			std::cout << "\n[Error Memory Unit Type] \n";
			std::cout << "마! 니 뭐여? -> " <<
				static_cast<int>(reinterpret_cast<MemoryUnit*>(pMemoryUnit)->memoryUnitType)
				<< std::endl;
		}
#pragma endregion
	}
}

/*
	GameServer::AfterRecv(SocketInfo* pClient)
		- 리시브 함수 호출 후, 클라이언트의 데이터를 실제로 받았을 때, 호출되는 함수.
*/
void GameServer::AfterRecv(SocketInfo* pClient, int cbTransferred)
{
	// 받은 데이터 처리
	ProcessRecvData(pClient, cbTransferred);

	// 바로 다시 Recv!
	NETWORK_UTIL::RecvPacket(pClient);
}

/*
	GameServer::ProcessRecvData(SocketInfo* pClient, int restSize)
		- 받은 데이터들을 패킷화하여 처리하는 함수.
*/
void GameServer::ProcessRecvData(SocketInfo* pClient, int restSize)
{
	char *pBuf = pClient->memoryUnit.dataBuf; // pBuf -> 처리하는 문자열의 시작 위치
	char packetSize{ 0 }; // 처리해야할 패킷의 크기

	// 이전에 처리를 마치지 못한 버퍼가 있다면, 지금 처리해야할 패킷 사이즈를 알려줘.
	if (0 < pClient->loadedSize ) packetSize = pClient->loadedBuf[0];

	// 처리하지 않은 버퍼의 크기가 있으면, 계속 루프문을 돕니다.
	while (restSize > 0)
	{
		// 이전에 처리를 마치지 못한 버퍼를 처리해야한다면 패스, 아니라면 지금 처리해야할 패킷의 크기를 받음.
		if (packetSize == 0) packetSize = static_cast<int>(pBuf[0]);

		// 처리해야하는 패킷 사이즈 중에서, 이전에 이미 처리한 패킷 사이즈를 빼준다.
		int required = packetSize - pClient->loadedSize;

		// 패킷을 완성할 수 있을 때 (요청해야할 사이즈보다, 남은 사이즈가 크거나 같을 때)
		if (restSize >= required)
		{
			memcpy(pClient->loadedBuf + pClient->loadedSize, pBuf, required);
			
			//-------------------------------------------------------------------------------
			zone->ProcessPacket(pClient); //== pClient->pZone->ProcessPacket(pClient); // 패킷처리 가가가가아아아아즈즈즈즞즈아아아아앗!!!!!!
			//-------------------------------------------------------------------------------

			pClient->loadedSize = 0;
			restSize -= required;
			pBuf += required;
			packetSize = 0;
		}
		// 패킷을 완성할 수 없을 때
		else
		{
			memcpy(pClient->loadedBuf + pClient->loadedSize , pBuf, restSize);
			pClient->loadedSize += restSize;
			break;
			//restSize = 0; 
		}
	}
}

/*
	GameServer::AfterSend(SocketInfo* pClient)
		- WSASend 함수 호출 후, 데이터 전송이 끝났을 때, 호출되는 함수.
*/
void GameServer::AfterSend(SendMemoryUnit* pMemoryUnit)
{
	// 보낼 때 사용한 버퍼 후처리하고 끝! ( 오버랩 초기화는 보낼떄 처리)
	SendMemoryPool::GetInstance()->PushMemory(pMemoryUnit);
}

void GameServer::ProcessTimerUnit(const int timerManagerContIndex)
{
	zone->ProcessTimerUnit(timerManagerContIndex);

	// 재활용하고 싶은건 재활용하고, 반납할건 반납하게하기 위해, 내부에서 정하도록 변경함.
	//TimerManager::GetInstance()->PushTimerUnit(pUnit);
}

#ifndef DISABLED_UNALLOCATED_MEMORY_SEND
void GameServer::AfterUnallocatedSend(UnallocatedMemoryUnit* pUnit)
{
	SendMemoryPool::GetInstance()->PushUnallocatedMemory(pUnit);
}
#endif
