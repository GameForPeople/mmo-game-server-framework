#include "pch.h"

#include "SocketInfo.h"

#include "MoveManager.h"

#include "GameServer.h"

GameServer::GameServer(bool inNotUse)
	: wsa()
	, hIOCP()
	, listenSocket()
	, serverAddr()
	, recvOrSendArr()
	, recvFunctionArr()
	, moveManager()
{
	PrintServerInfoUI();
	InitManagers();
	InitFunctions();
	InitNetwork();

	GLOBAL_UTIL::ERROR_HANDLING::errorRecvOrSendArr[0] = GLOBAL_UTIL::ERROR_HANDLING::HandleRecvOrSendError;
	GLOBAL_UTIL::ERROR_HANDLING::errorRecvOrSendArr[1] = GLOBAL_UTIL::ERROR_HANDLING::NotError;
};

GameServer::~GameServer()
{
}

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
	GameServer::InitManagers()
		- GamsServer의 생성자에서 호출되며, 각 매니저들의 초기화를 담당합니다.
*/
void GameServer::InitManagers()
{
	moveManager = std::make_unique<MoveManager>();
}

/*
	GameServer::InitFunctions()
		- GamsServer의 생성자에서 호출되며, 게임과 관련된 데이터들의 초기화를 담당합니다.
*/
void GameServer::InitFunctions()
{
	recvOrSendArr[NETWORK_TYPE::RECV] = &GameServer::AfterRecv;
	recvOrSendArr[NETWORK_TYPE::SEND] = &GameServer::AfterSend;

	recvFunctionArr[PACKET_TYPE::MOVE] = &GameServer::RecvCharacterMove;
}

/*
	GameServer::InitNetwork()
		- GamsServer의 생성자에서 호출되며, 네트워크 통신과 관련된 초기화를 담당합니다.
*/
void GameServer::InitNetwork()
{
	using namespace GLOBAL_UTIL::ERROR_HANDLING;

	//윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) ERROR_QUIT(TEXT("WSAStartup()"));

	// 입출력 완료 포트 생성
	if (hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)
		; hIOCP == NULL) ERROR_QUIT(TEXT("Make_WorkerThread()"));

	// CPU 개수 확인할 필요 없음.
	//SYSTEM_INFO si;
	//GetSystemInfo(&si);

	// 쓰레드 수 제어.
	HANDLE hThread;
	for (int i = 0; i < /* (int)si.dwNumberOfProcessors * 2 */ 2; ++i)
	{
		if (hThread = CreateThread(NULL, 0, StartWorkerThread, (LPVOID)this, 0, NULL)
			; hThread == NULL) ERROR_QUIT(TEXT("Make_WorkerThread()"));
		
		CloseHandle(hThread);
	}

	//Socket()
	if (listenSocket = socket(AF_INET, SOCK_STREAM, 0)
		; listenSocket == INVALID_SOCKET) ERROR_QUIT(TEXT("socket()"));

	//bind()
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERVER_PORT);

	if (int bindResult = ::bind(listenSocket, (SOCKADDR *)&serverAddr, sizeof(serverAddr))
		; bindResult == SOCKET_ERROR) ERROR_QUIT(TEXT("bind()"));

	// Listen()!
	if (int listenResult = listen(listenSocket, SOMAXCONN)
		; listenResult == SOCKET_ERROR) ERROR_QUIT(TEXT("listen()"));
}

/*
	GameServer::Run()
		- Accept Process
*/
void GameServer::Run()
{
	printf("Game Server activated!\n\n");

	SOCKET clientSocket{};
	SOCKADDR_IN clientAddr{};
	
	int addrLength = sizeof(clientAddr);

	DWORD recvBytes{}, flags{};
	int retValBuffer{};

	while (7) {
		//accept()
		if (clientSocket = WSAAccept(listenSocket, (SOCKADDR *)&clientAddr, &addrLength, NULL, NULL); 
			clientSocket == INVALID_SOCKET)
		{
			GLOBAL_UTIL::ERROR_HANDLING::ERROR_QUIT(TEXT("accept()"));
			break;
		}

		// 클라이언트 서버에 접속(Accept) 함을 알림
		//printf("[TCP 서버] 클라이언트 접속 : IP 주소 =%s, Port 번호 = %d \n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		// 소켓과 입출력 완료 포트 연결
		CreateIoCompletionPort((HANDLE)clientSocket, hIOCP, clientSocket, 0);

		// 소켓 정보 구조체 할당
		SocketInfo *pClient = new SocketInfo;
		if (pClient == nullptr)
		{
			GLOBAL_UTIL::ERROR_HANDLING::ERROR_QUIT(TEXT("Make_SocketInfo()"));
			break;
		}

		ZeroMemory(&pClient->overlapped, sizeof(pClient->overlapped));

		pClient->sock = clientSocket;

		pClient->wsabuf.buf = pClient->buf;
		pClient->wsabuf.len = SocketInfo::BUFFER_MAX_SIZE;

		// 비동기 입출력의 시작
		flags = 0;
		retValBuffer = WSARecv(
			clientSocket, 
			&pClient->wsabuf,
			1,			 
			NULL,		//NULL안하면 골치아파짐...!
			&flags,		 
			&pClient->overlapped, 
			NULL			// 능력부족으로 이벤트 방식 사용...
		);

		if (retValBuffer == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				GLOBAL_UTIL::ERROR_HANDLING::ERROR_DISPLAY(("WSARecv()"));
			}
			continue;
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
			std::cout << " [GoodBye] 클라이언트님 잘가시게나 \n";

			closesocket(pClient->sock);

			delete pClient;
			break;
		}
#pragma endregion
		
		//std::cout << " [RecvOrSend] GetRecvOrSend는 " << GetRecvOrSend(pClient->buf[0]) << "\n";
		recvOrSendArr[GetRecvOrSend(pClient->buf[0])](*this, pClient);
	}
}

/*
	GameServer::AfterRecv(SocketInfo* pClient)
		- 리시브 함수 호출 후, 클라이언트의 데이터를 실제로 받았을 때, 호출되는 함수.
*/
void GameServer::AfterRecv(SocketInfo* pClient)
{
	// 받은 데이터 처리 및 보낼 데이터 준비
	recvFunctionArr[pClient->buf[0]](*this, pClient);

	// 데이터 전송 요청
#ifdef _DEV_MODE_
	std::cout << "[Send]보낼 준비된 버퍼는" << int(pClient->buf[0]) << "희망하는 방향은 : " << int(pClient->buf[1]) << "\n";
#endif

	// 오버랩 갱신.
	ZeroMemory(&pClient->overlapped, sizeof(pClient->overlapped));
	
	// 버퍼 바인드 및 사이즈 설정.
	//pClient->wsabuf.len = 2; // 현재 고정

	// 데이터 전송
	// Socket Error일때 반환값이 -1이기 때문에, 1을 더해서 0일때 SocketError관련 함수, 이외는 아무것도 안함.
	GLOBAL_UTIL::ERROR_HANDLING::errorRecvOrSendArr[
		static_cast<bool>(
			1 + WSASend(pClient->sock, &pClient->wsabuf, 1, NULL, 0, &pClient->overlapped, NULL)
			)
	]();
}

/*
	GameServer::AfterSend(SocketInfo* pClient)
		- WSASend 함수 호출 후, 데이터 전송이 끝났을 때, 호출되는 함수.
*/
void GameServer::AfterSend(SocketInfo* pClient)
{
	// 버퍼 바인드 및 사이즈 설정.
	//pClient->wsabuf.len = 2;	// 현재 고정

	DWORD flag{};

	// 데이터 수신상태로 변경
	// Socket Error일때 반환값이 -1이기 때문에, 1을 더해서 0일때 SocketError관련 함수, 이외는 아무것도 안함.
	GLOBAL_UTIL::ERROR_HANDLING::errorRecvOrSendArr[
		static_cast<bool>(
			1 + WSARecv(pClient->sock, &pClient->wsabuf, 1, NULL, &flag /* NULL*/, &pClient->overlapped, NULL)
			)
	]();
}

/*
	GameServer::RecvCharacterMove(SocketInfo* pClient)
		- 클라이언트로부터 CharacterMove를 받았을 경우, 호출되는 함수.
*/
void GameServer::RecvCharacterMove(SocketInfo* pClient)
{
#ifdef _DEV_MODE_
	std::cout << "[AfterRecv] 받은 버퍼는" << int(pClient->buf[0]) << "희망하는 방향은 : " << int(pClient->buf[1]) << "\n";
#endif

	moveManager->MoveCharacter(pClient);
	moveManager->SendMoveCharacter(pClient);
	pClient->buf[0] = MakeSendPacket(PACKET_TYPE::MOVE);
}
