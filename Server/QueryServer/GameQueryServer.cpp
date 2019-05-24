#include "pch.h"
#include "../Define.h"
#include "GameQueryServer.h"

GameQueryServer::GameQueryServer(bool inNotUse)
	: wsa()
	, hIOCP()
	, listenSocket()
	, serverAddr()
	, workerThreadCont()
	, zoneCont()
	, chatManager(std::make_unique<ChatManager>())
{
	ServerIntegrityCheck();

	SendMemoryPool::MakeInstance();

	PrintServerInfoUI();
	InitNetwork();
};

GameQueryServer::~GameQueryServer()
{
	SendMemoryPool::DeleteInstance();

	workerThreadCont.clear();

	closesocket(listenSocket);
	CloseHandle(hIOCP);
}

void GameQueryServer::ServerIntegrityCheck()
{
	//무결성 검사
	static_assert(PACKET_TYPE::CHAT_TO_CLIENT::CHAT == PACKET_TYPE::CLIENT_TO_CHAT::CHAT,
		"CS::CHAT와 SC::CHAT의 값이 다르며, 이는 클라이언트의 채팅기능에 치명적인 오류를 발생시킵니다. 채팅 서버 실행을 거절하였습니다.");
}

/*
	GameServer::PrintServerInfoUI()
		- GamsServer의 생성자에서 호출되며, 서버의 UI들을 출력합니다.
*/
void GameQueryServer::PrintServerInfoUI()
{
	printf("\n■■■■■■■■■■■■■■■■■■■■■■■■■\n");
	printf("■ 게임서버프로그래밍 - 쿼리 서버 \n");
	printf("■                   게임공학과 원성연 2013182027\n");
	printf("■\n");

	// 추후 퍼블릭 IP로 변경.
	printf("■ IP : LocalHost(127.0.0.1)\n");
	printf("■ Listen Port : 9001\n");
	printf("■■■■■■■■■■■■■■■■■■■■■■■■■\n");
}

/*
	GameServer::InitNetwork()
		- GamsServer의 생성자에서 호출되며, 네트워크 통신과 관련된 초기화를 담당합니다.
*/
void GameQueryServer::InitNetwork()
{
	using namespace ERROR_HANDLING;

	// 1. 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) ERROR_QUIT(L"WSAStartup()");

	// 2. 입출력 완료 포트 생성
	if (hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)
		; hIOCP == NULL) ERROR_QUIT(TEXT("Create_IOCompletionPort()"));

	// 현재는 CPU 개수 확인할 필요 없음.
	//SYSTEM_INFO si;
	//GetSystemInfo(&si);

	// 3. 워커 쓰레드 생성 및 IOCP 등록.
	workerThreadCont.reserve(2);
	for (int i = 0; i < /* (int)si.dwNumberOfProcessors * 2 */ 2; ++i)
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
	serverAddr.sin_port = htons(GLOBAL_DEFINE::CHAT_SERVER_PORT);

	// 6. 소켓 설정
	if (::bind(listenSocket, (SOCKADDR *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) ERROR_QUIT(TEXT("bind()"));

	// 7. Listen()!
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) ERROR_QUIT(TEXT("listen()"));
}

/*
	GameServer::Run()
		- Accept Process 실행 및 Worker Thread join!
*/
void GameQueryServer::Run()
{
	std::thread acceptThread{ StartAcceptThread, (LPVOID)this };
	printf("Game Server activated!\n\n");

	acceptThread.join();
	for (auto& thread : workerThreadCont) thread.join();
}

/*
	GameServer::StartAcceptThread(LPVOID arg)
		- 쓰레드에서 멤버 변수를 사용하기 위해, 클래스 내부에서 엑셉트 쓰레드에 필요한 함수를 호출.
*/
DWORD WINAPI GameQueryServer::StartAcceptThread(LPVOID arg)
{
	GameQueryServer* pServer = static_cast<GameQueryServer*>(arg);
	pServer->AcceptThreadFunction();

	return 0;
};

/*
	GameServer::AcceptThreadFunction()
		- Accept를 담당합니다.

		#0. InNewClient에서 SocketInfo 할당이 이루어집니다.
		#1. InNewClient의 first가 false일 경우는, 동접보다 많은 수의 플레이어가 접속하려할 때 입니다.
*/
void GameQueryServer::AcceptThreadFunction()
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

		SocketInfo* pClient = new SocketInfo();
		pClient->sock = clientSocket;
		pClient->memoryUnit.wsaBuf.buf = pClient->memoryUnit.dataBuf;
		pClient->memoryUnit.wsaBuf.len = GLOBAL_DEFINE::MAX_SIZE_OF_RECV;

		// 소켓과 입출력 완료 포트 연결
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), hIOCP, pClient->sock, 0);

		//printf("[TCP 서버] 클라이언트 접속 : IP 주소 =%s, Port 번호 = %d \n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		std::cout << " [HELLO] 클라이언트 (" << inet_ntoa(clientAddr.sin_addr) << ") 가 접속했습니다. \n";

		// 비동기 입출력의 시작.
		NETWORK_UTIL::RecvPacket(pClient);

		// 접속을 받지 못하는 경우는, 염두에 두지 않음.
		//else {
		//	closesocket(clientSocket);
		//	//delete pClient;	// if nullptr;
		//}
	}
}

/*
	GameServer::StartWorkerThread(LPVOID arg)
		- 쓰레드에서 멤버 변수를 사용하기 위해, 클래스 내부에서 워커쓰레드에 필요한 함수를 호출.
*/
DWORD WINAPI GameQueryServer::StartWorkerThread(LPVOID arg)
{
	GameQueryServer* pServer = static_cast<GameQueryServer*>(arg);
	pServer->WorkerThreadFunction();

	return 0;
};

/*
	GameServer::WorkerThreadFunction()
		- 워커 쓰레드 함수.
*/
void GameQueryServer::WorkerThreadFunction()
{
	// 한 번만 선언해서 여러번 씁시다. 아껴써야지...
	int retVal{};
	DWORD cbTransferred;
	unsigned long long clientKey;

	LPVOID pMemoryUnit;

	while (7)
	{
#pragma region [ Wait For Event ]
		// 입출력 완료 포트에 저장된 결과를 처리하기 위한 함수 // 대기 상태가 됨
		retVal = GetQueuedCompletionStatus(
			hIOCP, //입출력 완료 포트 핸들
			&cbTransferred, //비동기 입출력 작업으로, 전송된 바이트 수가 여기에 저장된다.
			&clientKey, //함수 호출 시 전달한 세번째 인자(32비트) 가 여기에 저장된다.
			reinterpret_cast<LPOVERLAPPED *>(&pMemoryUnit), //Overlapped 구조체의 주소값
			INFINITE // 대기 시간 -> 깨울 까지 무한대
		);
#pragma endregion

#pragma region [ Error Exception ]
		//ERROR_CLIENT_DISCONNECT:
		if (retVal == 0 || cbTransferred == 0)
		{
			NETWORK_UTIL::LogOutProcess(pMemoryUnit);
			continue;
		}
#pragma endregion

		reinterpret_cast<MemoryUnit *>(pMemoryUnit)->memoryUnitType == MEMORY_UNIT_TYPE::RECV
			? AfterRecv(reinterpret_cast<SocketInfo*>(pMemoryUnit), cbTransferred)
			: AfterSend(reinterpret_cast<SendMemoryUnit*>(pMemoryUnit));

#ifndef  DISABLED_FUNCTION_POINTER
		//switch (reinterpret_cast<UnallocatedMemoryUnit*>(pMemoryUnit)->memoryUnitType)
		//{
		//case MEMORY_UNIT_TYPE::SEND:
		//	AfterSend(reinterpret_cast<SendMemoryUnit*>(pMemoryUnit));
		//	break;
		//case MEMORY_UNIT_TYPE::RECV:
		//	AfterRecv(reinterpret_cast<SocketInfo*>(pMemoryUnit), cbTransferred);
		//	break;
		//case MEMORY_UNIT_TYPE::UNALLOCATED_SEND:
		//	AfterUnallocatedSend(reinterpret_cast<UnallocatedMemoryUnit*>(pMemoryUnit));
		//	break;
		//}
#endif // ! DISABLED_FUNCTION_POINTER
	}
}

/*
	GameServer::AfterRecv(SocketInfo* pClient)
		- 리시브 함수 호출 후, 클라이언트의 데이터를 실제로 받았을 때, 호출되는 함수.
*/
void GameQueryServer::AfterRecv(SocketInfo* pClient, int cbTransferred)
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
void GameQueryServer::ProcessRecvData(SocketInfo* pClient, int restSize)
{
	char *pBuf = pClient->memoryUnit.dataBuf; // pBuf -> 처리하는 문자열의 시작 위치
	char packetSize{ 0 }; // 처리해야할 패킷의 크기

	// 이전에 처리를 마치지 못한 버퍼가 있다면, 지금 처리해야할 패킷 사이즈를 알려줘.
	if (0 < pClient->loadedSize) packetSize = pClient->loadedBuf[0];

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
			ProcessPacket(pClient); //== pClient->pZone->ProcessPacket(pClient); // 패킷처리 가가가가아아아아즈즈즈즞즈아아아아앗!!!!!!
			//-------------------------------------------------------------------------------

			pClient->loadedSize = 0;
			restSize -= required;
			pBuf += required;
			packetSize = 0;
		}
		// 패킷을 완성할 수 없을 때
		else
		{
			memcpy(pClient->loadedBuf + pClient->loadedSize, pBuf, restSize);
			pClient->loadedSize += restSize;
			break;
		}
	}
}

/*
	GameServer::AfterSend(SocketInfo* pClient)
		- WSASend 함수 호출 후, 데이터 전송이 끝났을 때, 호출되는 함수.
*/
void GameQueryServer::AfterSend(SendMemoryUnit* pMemoryUnit)
{
	// 보낼 때 사용한 버퍼 후처리하고 끝! ( 오버랩 초기화는 보낼떄 처리)
	SendMemoryPool::GetInstance()->PushMemory(pMemoryUnit);
}

void GameQueryServer::ProcessPacket(SocketInfo* pClient)
{
	using namespace PACKET_TYPE;

	switch (pClient->loadedBuf[1])
	{
	case CLIENT_TO_CHAT::CHAT:
		ProcessChat(pClient);
		break;
	case CLIENT_TO_CHAT::CONNECT:
		ProcessConnect(pClient);
		break;
	case CLIENT_TO_CHAT::CHANGE:
		ProcessChat(pClient);
		break;
	default:
		break;
	}
}

void GameQueryServer::ProcessChat(SocketInfo* pClient)
{
	//chatManager->ChatProcess(pClient, zoneCont);
}

void GameQueryServer::ProcessConnect(SocketInfo* pClient)
{

}

void GameQueryServer::ProcessChange(SocketInfo* pClient)
{

}
