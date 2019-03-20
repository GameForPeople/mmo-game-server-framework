#include "pch.h"

#include "SocketInfo.h"

#include "WorldManager.h"

#include "GameServer.h"

GameServer* GameServer::instance = nullptr;

GameServer::GameServer()
	: wsa()
	, listenSocket()
	, serverAddr()
	//, recvOrSendArr()
	, recvFunctionArr()
	, worldManager()
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
	closesocket(listenSocket);
	WSACleanup();
}

void GameServer::PrintServerInfoUI() const
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
	worldManager = std::make_unique<WorldManager>();
}

/*
	GameServer::InitFunctions()
		- GamsServer의 생성자에서 호출되며, 게임과 관련된 데이터들의 초기화를 담당합니다.
*/
void GameServer::InitFunctions()
{
	//recvOrSendArr[NETWORK_TYPE::RECV] = &GameServer::AfterRecv;
	//recvOrSendArr[NETWORK_TYPE::SEND] = &GameServer::AfterSend;

	recvFunctionArr[PACKET_TYPE::VOID_UPDATE] = &GameServer::RecvVoidUpdate;
	recvFunctionArr[PACKET_TYPE::MOVE] = &GameServer::RecvCharacterMove;
}

/*
	GameServer::InitNetwork()
		- GamsServer의 생성자에서 호출되며, 네트워크 통신과 관련된 초기화를 담당합니다.
*/
void GameServer::InitNetwork()
{
	using namespace GLOBAL_UTIL::ERROR_HANDLING;

	// 0. 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) ERROR_QUIT(TEXT("WSAStartup()"));

	// 1. 소켓 생성
	if (listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)
		; listenSocket == INVALID_SOCKET) ERROR_QUIT(TEXT("socket()"));

	// 2. 서버 정보 객체 설정
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERVER_PORT);

	// 3. 소켓 설정
	if (::bind(listenSocket, (SOCKADDR *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) ERROR_QUIT(TEXT("bind()"));

	// 4. 수신 대기열 설정
	if (listen(listenSocket, MAX_CLIENT_COUNT) == SOCKET_ERROR) ERROR_QUIT(TEXT("listen()"));
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

	while (7) 
	{
		//accept()
		if (clientSocket = WSAAccept(listenSocket, (SOCKADDR *)&clientAddr, &addrLength, NULL, NULL)
			; clientSocket == INVALID_SOCKET)
		{
			GLOBAL_UTIL::ERROR_HANDLING::ERROR_QUIT(TEXT("accept()"));
			break;
		}

		// 소켓 정보 구조체 할당
		SocketInfo* pTempClient = new SocketInfo;

		pTempClient->socket = clientSocket;
		pTempClient->wsabuf.buf = pTempClient->buf;
		pTempClient->wsabuf.len = 2;// SocketInfo::BUFFER_MAX_SIZE;

		// 중첩 소캣을 지정하고 완료시 실행될 함수를 넘겨준다.
		pTempClient->overlapped.hEvent = (HANDLE)(pTempClient->socket);

		clientCont[clientSocket] = pTempClient;

		// 비동기 입출력의 시작
		flags = 0;

#ifdef _DEV_MODE_
		if(clientCont[clientSocket] == nullptr) std::cout << " [SocketInfo] 널포인트 입니다. \n";
#endif
		std::cout << " [HELLO] 클라이언트 ( " << (int)clientSocket << ", " << inet_ntoa(clientAddr.sin_addr) << ") 가 접속했습니다. \n";

		GLOBAL_UTIL::ERROR_HANDLING::errorRecvOrSendArr[
			static_cast<bool>(1 + WSARecv(clientCont[clientSocket]->socket, &(clientCont[clientSocket]->wsabuf), 1, NULL,
				&flags,	(&clientCont[clientSocket]->overlapped), CallBackRecv))
		]();
	}
}


void CALLBACK GameServer::CallBackRecv(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	GameServer::GetInstance()->CallBackRecvProcess(dataBytes, overlapped);
}

void GameServer::CallBackRecvProcess(DWORD dataBytes, LPWSAOVERLAPPED overlapped)
{
	SOCKET clientSocket = reinterpret_cast<int>(overlapped->hEvent);
	SocketInfo* pTempClient = clientCont[clientSocket];

#ifdef _DEV_MODE_
	if (pTempClient == nullptr) std::cout << " [SocketInfo] 널포인트 입니다. \n";
#endif

	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	if (dataBytes == 0)
	{
		SOCKADDR_IN clientAddr;
		int addrLength = sizeof(clientAddr);
		getpeername(pTempClient->socket, (SOCKADDR *)&clientAddr, &addrLength);

		std::cout << " [GOODBYE] 클라이언트 ( " << (int)clientSocket << ", " << inet_ntoa(clientAddr.sin_addr) << ") 가 종료했습니다. \n";

		// 클라이언트가 closesocket을 함.
		closesocket(pTempClient->socket);
		clientCont.erase(clientSocket);

		delete pTempClient;
		return;
	}  

#ifdef _DEV_MODE_
	std::cout << "\n"<< (int)pTempClient->socket <<" : [Recv] 받은 버퍼는 PK_MOVE(100) 희망하는 방향은 : " <<
		[/*void*/](const BYTE dir) noexcept -> std::string
	{
		if (dir == DIRECTION::LEFT) return "LEFT";
		if (dir == DIRECTION::RIGHT) return "RIGHT";
		if (dir == DIRECTION::UP) return "UP";
		if (dir == DIRECTION::DOWN) return "DOWN";

	}(int(pTempClient->buf[1]))
		<< "\n";
#endif

	// 받은 데이터 처리 및 보낼 데이터 준비
	recvFunctionArr[(pTempClient->buf[0]) % (PACKET_TYPE::ENUM_SIZE)](*this, (clientCont[clientSocket]));

	// 오버랩 갱신.
	ZeroMemory(&(pTempClient->overlapped), sizeof(pTempClient->overlapped));
	pTempClient->overlapped.hEvent = (HANDLE)clientSocket;

	// 데이터 전송 요청
#ifdef _DEV_MODE_
	std::cout << (int)pTempClient->socket << " : [Send] 전송 준비된 버퍼는" << int(pTempClient->buf[0]) << "내용은 : "
		<< "\n 1 : " << std::bitset<8>(pTempClient->buf[1])
		<< "\n 2 : " << std::bitset<8>(pTempClient->buf[2])
		<< "\n 3 : " << std::bitset<8>(pTempClient->buf[3])
		<< "\n 4 : " << std::bitset<8>(pTempClient->buf[4])
		<< "\n 5 : " << std::bitset<8>(pTempClient->buf[5])
		<< "\n 6 : " << std::bitset<8>(pTempClient->buf[6])
		<< "\n 7 : " << std::bitset<8>(pTempClient->buf[7])
		<< "\n 8 : " << std::bitset<8>(pTempClient->buf[8])
		<< "\n 9 : " << std::bitset<8>(pTempClient->buf[9])
		<< "\n 10 : " << std::bitset<8>(pTempClient->buf[10])
		<< "\n 11 : " << std::bitset<8>(pTempClient->buf[11])
		<< "\n 12 : " << std::bitset<8>(pTempClient->buf[12]) << "\n";
#endif

	// 버퍼 바인드 및 사이즈 설정.
	pTempClient->wsabuf.len = SocketInfo::BUFFER_MAX_SIZE; // 현재 고정

	// 데이터 전송
	// Socket Error일때 반환값이 -1이기 때문에, 1을 더해서 0일때 SocketError관련 함수, 이외는 아무것도 안함.
	GLOBAL_UTIL::ERROR_HANDLING::errorRecvOrSendArr[
		static_cast<bool>(
			1 + WSASend(pTempClient->socket, &pTempClient->wsabuf, 1, &dataBytes, 0, &pTempClient->overlapped, CallBackSend)
			)
	]();
}

void CALLBACK GameServer::CallBackSend(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	GameServer::GetInstance()->CallBackSendProcess(dataBytes, overlapped);
}

void GameServer::CallBackSendProcess(DWORD dataBytes, LPWSAOVERLAPPED overlapped)
{
	SOCKET clientSocket = reinterpret_cast<int>(overlapped->hEvent);
	SocketInfo* pTempClient = clientCont[clientSocket];
	
#ifdef _DEV_MODE_
	if (pTempClient == nullptr) std::cout << " [SocketInfo] 널포인트 입니다. \n";
#endif

	if (dataBytes == 0)
	{
		SOCKADDR_IN clientAddr;
		int addrLength = sizeof(clientAddr);
		getpeername(pTempClient->socket, (SOCKADDR *)&clientAddr, &addrLength);

		std::cout << " [GOODBYE] 클라이언트 ( " << (int)clientSocket << ", " << inet_ntoa(clientAddr.sin_addr) << ") 가 종료했습니다. \n";

		// 클라이언트가 closesocket을 함.
		closesocket(pTempClient->socket);
		clientCont.erase(clientSocket);

		delete pTempClient;
		return;
	}

	DWORD flag{};

	ZeroMemory(&(pTempClient->overlapped), sizeof(pTempClient->overlapped));
	pTempClient->overlapped.hEvent = (HANDLE)clientSocket;
	pTempClient->wsabuf.len = 2;

	GLOBAL_UTIL::ERROR_HANDLING::errorRecvOrSendArr[
		static_cast<bool>(
			1 + WSARecv(pTempClient->socket, &pTempClient->wsabuf, 1, &dataBytes, &flag /* NULL*/, &pTempClient->overlapped, CallBackRecv)
			)
	]();
}



void GameServer::RecvVoidUpdate(SocketInfo* pClient)
{
	pClient->buf[0] = /* MakeSendPacket*/(PACKET_TYPE::VOID_UPDATE);

	LoadOtherPlayerPositionToSendBuffer(pClient, 1);
}

/*
	GameServer::RecvCharacterMove(SocketInfo* pClient)
		- 클라이언트로부터 CharacterMove를 받았을 경우, 호출되는 함수.
*/
void GameServer::RecvCharacterMove(SocketInfo* pClient)
{
	worldManager->MoveCharacter(pClient);
	worldManager->SendMoveCharacter(pClient);
	pClient->buf[0] = /* MakeSendPacket*/(PACKET_TYPE::MOVE);

	LoadOtherPlayerPositionToSendBuffer(pClient, 2);
}

void GameServer::LoadOtherPlayerPositionToSendBuffer(SocketInfo* pClient, const int inStartIndex)
{
	pClient->buf[inStartIndex] = clientCont.size();

	int tempIndex{ 0 };
	for (auto iter = clientCont.begin(); iter != clientCont.end(); ++iter)
	{
		if (iter->first != pClient->socket)
		{
			pClient->buf[inStartIndex + ++tempIndex ] =
				GLOBAL_UTIL::BIT_CONVERTER::MakeByteFromLeftAndRightByte
				(
					iter->second->userData->GetPosition().x,
					iter->second->userData->GetPosition().y
				);
		}
	}
}
