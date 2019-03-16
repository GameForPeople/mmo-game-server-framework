#include "pch.h"

#include "SocketInfo.h"
#include "PacketType.h"

#include "MoveManager.h"

#include "GameServer.h"

GameServer::GameServer()
	: wsa()
	, hIOCP()
	, listenSocket()
	, serverAddr()
	, recvOrSend()
	, recvFunctionArr()
	, moveManager()
{
	InitManagers();
	InitFunctions();
	InitNetwork();
};

GameServer::~GameServer()
{
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
	recvOrSend[0] = &GameServer::AfterRecv;
	recvOrSend[1] = &GameServer::AfterSend;

	recvFunctionArr[0] = &GameServer::RecvCharacterMove;
}

/*
	GameServer::InitNetwork()
		- GamsServer의 생성자에서 호출되며, 네트워크 통신과 관련된 초기화를 담당합니다.
*/
void GameServer::InitNetwork()
{
	using namespace NETWORK_UTIL::ERROR_HANDLING;

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
			NETWORK_UTIL::ERROR_HANDLING::ERROR_QUIT(TEXT("accept()"));
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
			NETWORK_UTIL::ERROR_HANDLING::ERROR_QUIT(TEXT("Make_SocketInfo()"));
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
				NETWORK_UTIL::ERROR_HANDLING::ERROR_DISPLAY(("WSARecv()"));
			}
			continue;
		}
	}
}

DWORD WINAPI GameServer::StartWorkerThread(LPVOID arg)
{
	GameServer* pServer = static_cast<GameServer*>(arg);
	pServer->WorkerThreadFunction();

	return 0;
};

void GameServer::WorkerThreadFunction()
{
	// 한 번만 선언해서 여러번 씁시다. 아껴써야지...
	int retVal{};
	DWORD cbTransferred;

	while (7)
	{
#pragma region [ Wait For Thread ]
		//비동기 입출력 기다리기
		SOCKET clientSocket;
		SocketInfo *pClient;

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
		// 할당받은 소켓 즉! 클라이언트 정보 얻기
		
		//SOCKADDR_IN clientAddr;
		//int addrLength = sizeof(clientAddr);
		//getpeername(pClient->sock, (SOCKADDR *)&clientAddr, &addrLength);

	//ERROR_CLIENT_DISCONNECT:
		if (retVal == 0 || cbTransferred == 0)
		{
			closesocket(pClient->sock);

			delete pClient;
			break;
		}
#pragma endregion
		
		std::cout << "받은 버퍼는" << int(pClient->buf[0]) << "\n";
		std::cout << "GetRecvOrSend는 " << GetRecvOrSend(pClient->buf[0]) << "\n";

		recvOrSend[GetRecvOrSend(pClient->buf[0])](*this, pClient);
	}
}

void GameServer::AfterRecv(SocketInfo* pClient)
{
	// 받은 데이터 처리 및 보낼 데이터 준비
	recvFunctionArr[pClient->buf[0]](*this, pClient);

	// 데이터 전송 요청

	// 오버랩 갱신.
	ZeroMemory(&pClient->overlapped, sizeof(pClient->overlapped));
	
	// 데이터 바인드
	pClient->wsabuf.buf = pClient->buf;
	pClient->wsabuf.len = 2;

	// 데이터 전송
	WSASend(pClient->sock, &pClient->wsabuf, 1, NULL, 0, &pClient->overlapped, NULL);
}

void GameServer::AfterSend(SocketInfo* pClient)
{
	// 데이터 전송 완료됨을 확인함.

	pClient->wsabuf.buf = pClient->buf;
	pClient->wsabuf.len = 2;

	// 데이터 수신상태로 변경
	WSARecv(pClient->sock, &pClient->wsabuf, 1, /*&recvBytes*/ NULL, /*&flags*/ NULL, &pClient->overlapped, NULL);
}

void GameServer::RecvCharacterMove(SocketInfo* pClient)
{
	moveManager->MoveCharacter(pClient);
	moveManager->SendMoveCharacter(pClient);
	pClient->buf[0] = MakeSendPacket(PACKET_TYPE::MOVE);

	std::cout << "보내는 버퍼는"<< int(BYTE(PACKET_TYPE::MOVE)) << " " << int(pClient->buf[0]) << "\n";
}
