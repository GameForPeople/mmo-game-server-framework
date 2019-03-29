#include "pch.h"
#include "Define.h"
#include "ServerDefine.h"

#include "SocketInfo.h"

#include "Scene.h"
#include "SendMemoryPool.h"

#include "GameServer.h"

GameServer::GameServer(bool inNotUse)
	: wsa()
	, hIOCP()
	, listenSocket()
	, serverAddr()
	//, recvOrSendArr()
	, workerThreadCont()
	, sceneCont()
{
	PrintServerInfoUI();
	InitScenes();
	InitFunctions();
	InitNetwork();

	SendMemoryPool::MakeInstance();

	ERROR_HANDLING::errorRecvOrSendArr[0] = ERROR_HANDLING::HandleRecvOrSendError;
	ERROR_HANDLING::errorRecvOrSendArr[1] = ERROR_HANDLING::NotError;
};

GameServer::~GameServer()
{
	SendMemoryPool::DeleteInstance();

	workerThreadCont.clear();
	sceneCont.clear();

	CloseHandle(hIOCP);
}

/*
	GameServer::PrintServerInfoUI()
		- GamsServer의 생성자에서 호출되며, 서버의 UI들을 출력합니다.
*/
void GameServer::PrintServerInfoUI()
{
	printf("\n■■■■■■■■■■■■■■■■■■■■■■■■■\n");
	printf("■ 게임서버프로그래밍 숙제 2번   \n");
	printf("■                   게임공학과 원성연 2013182027\n");
	printf("■\n");
	
	// 추후 퍼블릭 IP로 변경.
	printf("■ IP : LocalHost(127.0.0.1)\n");
	printf("■ Listen Port : 9000\n");
	printf("■■■■■■■■■■■■■■■■■■■■■■■■■\n");
}

/*
	GameServer::InitScene()
		- GamsServer의 생성자에서 호출되며, 씐들의 초기화를 담당합니다.
*/
void GameServer::InitScenes()
{
	sceneCont.reserve(1);
	sceneCont.emplace_back(std::make_unique<Scene>());
}

/*
	GameServer::InitFunctions()
		- GamsServer의 생성자에서 호출되며, 함수 포인터들의 초기화를 담당합니다.
*/
void GameServer::InitFunctions()
{
#ifdef DISABLED_FUNCTION_POINTER
#else
	recvOrSendArr = new std::function <void(GameServer&, LPVOID)>[NETWORK_TYPE::ENUM_SIZE];
	recvOrSendArr[NETWORK_TYPE::RECV] = &GameServer::AfterRecv;
	recvOrSendArr[NETWORK_TYPE::SEND] = &GameServer::AfterSend;
#endif
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
		; hIOCP == NULL) ERROR_QUIT(TEXT("Make_WorkerThread()"));

	// CPU 개수 확인할 필요 없음.
	//SYSTEM_INFO si;
	//GetSystemInfo(&si);

	// 3. 워커 쓰레드 생성 및 IOCP 등록.
	workerThreadCont.reserve(2);
	for (int i = 0; i < /* (int)si.dwNumberOfProcessors * 2 */ 2; ++i)
	{
		workerThreadCont.emplace_back(std::thread{ StartWorkerThread, (LPVOID)this });
		//if (hThread = CreateThread(NULL, 0, StartWorkerThread, (LPVOID)this, 0, NULL)
		//	; hThread == NULL) ERROR_QUIT(TEXT("Make_WorkerThread()"));
		//
		//CloseHandle(hThread);
	}

	// 4. 소켓 생성
	if (listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)
		; listenSocket == INVALID_SOCKET) ERROR_QUIT(TEXT("socket()"));

	// 5. 서버 정보 객체 설정
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(GLOBAL_DEFINE::SERVER_PORT);

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
	std::thread acceptThread{ StartAcceptThread, (LPVOID)this };
	printf("Game Server activated!\n\n");
	
	acceptThread.join();
	for (auto& thread : workerThreadCont) thread.join();
}

/*
	GameServer::StartAcceptThread(LPVOID arg)
		- 쓰레드에서 멤버 변수를 사용하기 위해, 클래스 내부에서 엑셉트 쓰레드에 필요한 함수를 호출.
*/
DWORD WINAPI GameServer::StartAcceptThread(LPVOID arg)
{
	GameServer* pServer = static_cast<GameServer*>(arg);
	pServer->AcceptThreadFunction();

	return 0;
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

		if (auto [isTrueAdd, pClient] = sceneCont[0]->InNewClient()
			; isTrueAdd)
		{
			// 소켓과 입출력 완료 포트 연결
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), hIOCP, pClient->clientContIndex, 0);

			pClient->sock = clientSocket;
			pClient->wsaBuf.buf = pClient->recvBuf;
			pClient->wsaBuf.len = GLOBAL_DEFINE::MAX_SIZE_OF_RECV;

			// 클라이언트 서버에 접속(Accept) 함을 알림
			//printf("[TCP 서버] 클라이언트 접속 : IP 주소 =%s, Port 번호 = %d \n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
			std::cout << " [HELLO] 클라이언트 (" << inet_ntoa(clientAddr.sin_addr) << ") 가 접속했습니다. \n";
			
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
DWORD WINAPI GameServer::StartWorkerThread(LPVOID arg)
{
	GameServer* pServer = static_cast<GameServer*>(arg);
	pServer->WorkerThreadFunction();

	return 0;
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
	SOCKET clientSocket;
	SocketInfo *pClient;

	while (7)
	{
#pragma region [ Wait For Event ]
		// 입출력 완료 포트에 저장된 결과를 처리하기 위한 함수 // 대기 상태가 됨
		retVal = GetQueuedCompletionStatus(
			hIOCP, //입출력 완료 포트 핸들
			&cbTransferred, //비동기 입출력 작업으로, 전송된 바이트 수가 여기에 저장된다.
			&clientSocket, //함수 호출 시 전달한 세번째 인자(32비트) 가 여기에 저장된다.
			(LPOVERLAPPED *)&pClient, //Overlapped 구조체의 주소값
			INFINITE // 대기 시간 -> 깨울 까지 무한대
		);
#pragma endregion

#pragma region [ Error Exception ]
	//ERROR_CLIENT_DISCONNECT:
		if (retVal == 0 || cbTransferred == 0)
		{
			NETWORK_UTIL::LogOutProcess(pClient);
			break;
		}
#pragma endregion

#ifdef DISABLED_FUNCTION_POINTER
		BIT_CONVERTER::GetRecvOrSend(pClient->recvBuf[0]) == true
			? AfterSend(pClient)
			: AfterRecv(pClient, cbTransferred);
#else
		recvOrSendArr[GLOBAL_UTIL::BIT_CONVERTER::GetRecvOrSend(pClient->buf[0])](*this, pClient);
#endif
	}
}

/*
	GameServer::AfterRecv(SocketInfo* pClient)
		- 리시브 함수 호출 후, 클라이언트의 데이터를 실제로 받았을 때, 호출되는 함수.
*/
void GameServer::AfterRecv(LPVOID pClient, int cbTransferred)
{
	// 받은 데이터 처리
	ProcessRecvData(static_cast<SocketInfo*>(pClient), cbTransferred);

	// 바로 다시 Recv!
	NETWORK_UTIL::RecvPacket(static_cast<SocketInfo*>(pClient));
}

/*
	GameServer::ProcessRecvData(SocketInfo* pClient, int restSize)
		- 받은 데이터들을 패킷화하여 처리하는 함수.
*/
void GameServer::ProcessRecvData(SocketInfo* pClient, int restSize)
{
	char *pBuf = pClient->recvBuf; // pBuf -> 처리하는 문자열의 시작 위치
	char packetSize{ 0 }; // 처리해야할 패킷의 크기

	// 이전에 처리를 마치지 못한 버퍼가 있다면, 처리해야할 패킷 사이즈를 알려줘.
	if (0 < pClient->loadedSize ) packetSize = pClient->loadedBuf[0];

	// 처리하지 않은 버퍼의 크기가 0이 될때까지 돌립니다.
	while (restSize > 0)
	{
		// 이전에 처리를 마치지 못한 버퍼를 처리해야한다면 패스, 아니라면 처리해야할 패킷의 크기를 받음.
		if (packetSize == 0) packetSize = pBuf[0];

		// 처리해야하는 패킷 사이즈 중에서, 이전에 이미 처리한 패킷 사이즈를 빼준다.
		int required = packetSize - pClient->loadedSize;

		// 패킷을 완성할 수 있을 때 (요청해야할 사이즈보다, 남은 사이즈가 크거나 같을 때)
		if (restSize >= required)
		{
			memcpy(pClient->loadedBuf + pClient->loadedSize, pBuf, required);
			pClient->pScene->ProcessPacket(pClient);

			restSize -= required;
			pBuf += required;
			packetSize = 0;
		}
		// 패킷을 완성할 수 없을 때
		else
		{
			memcpy(pClient->loadedBuf, pBuf, restSize);
			pClient->loadedSize = restSize;
			restSize = 0;
		}
	}
}

/*
	GameServer::AfterSend(SocketInfo* pClient)
		- WSASend 함수 호출 후, 데이터 전송이 끝났을 때, 호출되는 함수.
*/
void GameServer::AfterSend(LPVOID pSendMemoryUnit)
{
	// 보낼 때 사용한 버퍼 후처리하고 끝! ( 오버랩 초기화는 보낼떄 처리)
	SendMemoryPool::GetInstance()->PushMemory(static_cast<SendMemoryUnit*>(pSendMemoryUnit));
}

