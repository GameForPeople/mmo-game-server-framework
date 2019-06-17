#include "stdafx.h"

#include "ClientDefine.h"
#include "Define.h"

#include "MemoryUnit.h"

#include "GameFrameWork.h"
#include "NetworkManager.h"

NetworkManager::NetworkManager(const std::string_view& inIPAddress, WGameFramework* InGameFramework)
	: ipAddress(inIPAddress) 
	, pGameFramework(InGameFramework)
	, hIOCP()
	, workerThread()
	, wsa()
	, socket()
	, serverAddr()
	, recvMemoryUnit(nullptr)
	, loadedBuf()
	, loadedSize()
{
	//for (int i = 0; i < 10; ++i)
	//{
	//	recvCharacterPoistionArr[i] = {0, 0};
	//}
	ERROR_HANDLING::errorRecvOrSendArr[0] = ERROR_HANDLING::HandleRecvOrSendError;
	ERROR_HANDLING::errorRecvOrSendArr[1] = ERROR_HANDLING::NotError;

	recvMemoryUnit = new MemoryUnit(true);

	InitNetwork();
}

NetworkManager::~NetworkManager()
{
	workerThread.join();

	pGameFramework = nullptr;

	delete recvMemoryUnit;
}

void NetworkManager::InitNetwork()
{
	using namespace GLOBAL_UTIL::ERROR_HANDLING;

	// 1. 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) ERROR_QUIT(L"WSAStartup()");

	// 2. 입출력 완료 포트 생성
	if (hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)
		; hIOCP == NULL) ERROR_QUIT(TEXT("Make_WorkerThread()"));

	// 3. 소캣 생성
	if (this->socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)
		; socket == INVALID_SOCKET) ERROR_QUIT(L"Create_Socket()");

	// 4. 소켓과 입출력 완료 포트 연결
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), hIOCP, socket, 0);

	// 5. 클라이언트 정보 구조체 객체 설정.
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(ipAddress.c_str());
	serverAddr.sin_port = htons(GLOBAL_DEFINE::MAIN_SERVER_PORT);

	// 6. 워커 쓰레드 생성 및 IOCP 등록
	workerThread = std::thread{ StartWorkerThread, (LPVOID)this };

	// 7. 커넥트!!!!!!!!! 가자아아아아아아아앗!!!!!!!
	if (int retVal = connect(socket, (SOCKADDR*)& serverAddr, sizeof(serverAddr))
		; retVal == SOCKET_ERROR) ERROR_QUIT(L"bind()");

	std::cout << "[CONNECT] 서버에 정상적으로 연결되었습니다." << std::endl;

	LogInOrSignUpProcess();

	// 8. 리시브 온!
	RecvPacket();
}

DWORD WINAPI NetworkManager::StartWorkerThread(LPVOID arg)
{
	NetworkManager* pNetworkManager = static_cast<NetworkManager*>(arg);
	pNetworkManager->WorkerThreadFunction();

	return 0;
};

void NetworkManager::WorkerThreadFunction()
{
	int retVal{};
	DWORD cbTransferred;
	unsigned long long clientKey;
	MemoryUnit* pMemoryUnit;

	while (7)
	{
		retVal = GetQueuedCompletionStatus(
			hIOCP,
			&cbTransferred,
			&clientKey,
			reinterpret_cast<LPOVERLAPPED*>(&pMemoryUnit),
			INFINITE
		);

		if (retVal == 0 || cbTransferred == 0)
		{
			// 아니! 클라이언트에서 네트워크 오류가 났다고??
			std::cout << "[오류] 네트워크 오류가 발생했습니다. 클라이언트를 종료합니다." << std::endl;
			
			if (retVal == 0)
			{
				std::cout << "[오류] 종류 원인은 retVal == 0입니다." << std::endl;
			}
			else
			{
				std::cout << "[오류] 종류 원인은 cbTransferred == 0입니다." << std::endl;
			}
			
			ERROR_HANDLING::ERROR_QUIT(L"GetQueuedCompletionStatus()");

			break;
		}

		pMemoryUnit->isRecv == true
			? AfterRecv(cbTransferred)
			: AfterSend(pMemoryUnit);
	}
}

/*
	SendPacket()
		- WSASend!는 여기에서만 존재할 수 있습니다.

	!0. 단순히 WSA Send만 있는 함수입니다. 데이터는 준비해주세요.
	!1. 이 함수를 호출하기 전에, wsaBuf의 len을 설정해주세요.

	?0. wsaBuf의 buf는 보낼때마다 바꿔줘야 할까요?
*/
void NetworkManager::SendPacket(char* packetData)
{
	MemoryUnit* sendMemoryUnit = new MemoryUnit(false);
	memcpy(sendMemoryUnit->dataBuf, packetData, packetData[0]);
	sendMemoryUnit->wsaBuf.len = packetData[0];

	DWORD flag{};
	ZeroMemory(&sendMemoryUnit->overlapped, sizeof(sendMemoryUnit->overlapped));

	ERROR_HANDLING::errorRecvOrSendArr[
		static_cast<bool>(
			1 + WSASend(socket, &sendMemoryUnit->wsaBuf, 1, NULL, 0, &sendMemoryUnit->overlapped, NULL)
			)
	]();
}

/*
	RecvPacket()
		- WSA Recv는 여기서만 존재합니다.

	!0. SocketInfo에 있는 wsaBuf -> buf 에 리시브를 합니다.
	!1. len은 고정된 값을 사용합니다. MAX_SIZE_OF_RECV_BUFFER!
*/
void NetworkManager::RecvPacket()
{
	// 받은 데이터에 대한 처리가 끝나면 바로 다시 받을 준비.
	DWORD flag{};
	
	ZeroMemory(&(recvMemoryUnit->overlapped), sizeof(recvMemoryUnit->overlapped));
	ERROR_HANDLING::errorRecvOrSendArr[
		static_cast<bool>(
			1 + WSARecv(socket, &(recvMemoryUnit->wsaBuf), 1, NULL, &flag /* NULL*/, &(recvMemoryUnit->overlapped), NULL)
			)
	]();
}

/*
	NetworkManager::AfterRecv(SocketInfo* pClient)
		- 리시브 함수 호출 후, 클라이언트의 데이터를 실제로 받았을 때, 호출되는 함수.
*/
void NetworkManager::AfterRecv(/*MemoryUnit* pClient,*/ int cbTransferred)
{
	//std::cout << "패킷 받음 size : " << (int)loadedBuf[0] << " Type : " << (int)loadedBuf[1] << std::endl;

	// 받은 데이터 처리
	ProcessRecvData(cbTransferred);

	RecvPacket();
}

/*
	NetworkManager::ProcessRecvData(SocketInfo* pClient, int restSize)
		- 받은 데이터들을 패킷화하여 처리하는 함수.
*/
void NetworkManager::ProcessRecvData(int restSize)
{
	char* pBuf = recvMemoryUnit->dataBuf; // pBuf -> 처리하는 문자열의 시작 위치
	char packetSize{ 0 }; // 처리해야할 패킷의 크기

	// 이전에 처리를 마치지 못한 버퍼가 있다면, 처리해야할 패킷 사이즈를 알려줘.
	if (0 < loadedSize) packetSize = loadedBuf[0];

	// 처리하지 않은 버퍼의 크기가 0이 될때까지 돌립니다.
	while (restSize > 0)
	{
		// 이전에 처리를 마치지 못한 버퍼를 처리해야한다면 패스, 아니라면 처리해야할 패킷의 크기를 받음.
		if (packetSize == 0) packetSize = pBuf[0];

		// 처리해야하는 패킷 사이즈 중에서, 이전에 이미 처리한 패킷 사이즈를 빼준다.
		int required = packetSize - loadedSize;

		// 패킷을 완성할 수 있을 때 (요청해야할 사이즈보다, 남은 사이즈가 크거나 같을 때)
		if (restSize >= required)
		{
			memcpy(loadedBuf + loadedSize, pBuf, required);
			//---

			ProcessLoadedPacket();

			//---
			loadedSize = 0;
			restSize -= required;
			pBuf += required;
			packetSize = 0;
		}
		// 패킷을 완성할 수 없을 때
		else
		{
			memcpy(loadedBuf + loadedSize, pBuf, restSize);
			loadedSize += restSize;
			break;
			//restSize = 0;
		}
	}
}

/*
	NetworkManager::ProcessLoadedPacket()
		- 받은 데이터들을 패킷화하여 처리하는 함수.
*/
void NetworkManager::ProcessLoadedPacket()
{
	using namespace PACKET_TYPE;

	switch (loadedBuf[1])
	{
	case MAIN_TO_CLIENT::LOGIN_OK:
		pGameFramework->RecvLoginOK(loadedBuf);
		break;
	case MAIN_TO_CLIENT::PUT_PLAYER:
		pGameFramework->RecvPutPlayer(loadedBuf);
		break;
	case MAIN_TO_CLIENT::REMOVE_PLAYER:
		pGameFramework->RecvRemovePlayer(loadedBuf);
		break;
	case MAIN_TO_CLIENT::POSITION:
		pGameFramework->RecvPosition(loadedBuf);
		break;
	case MAIN_TO_CLIENT::LOGIN_FAIL:
		pGameFramework->RecvLoginFail(loadedBuf);
		break;
	case MAIN_TO_CLIENT::CHAT:
		pGameFramework->RecvChat(loadedBuf);
		break;
	case MAIN_TO_CLIENT::STAT_CHANGE:
		pGameFramework->RecvStatChange(loadedBuf);
		break;
	default:
		std::cout << "[RECV] 정의되지 않은 프로토콜을 받았습니다. 확인해주세요. " << loadedBuf[1] << "\n";
		break;
	}
}

/*
	GameServer::AfterSend(SocketInfo* pClient)
		- WSASend 함수 호출 후, 데이터 전송이 끝났을 때, 호출되는 함수.
*/
void NetworkManager::AfterSend(MemoryUnit* pMemoryUnit)
{
	delete pMemoryUnit;
}

void NetworkManager::SendMoveData(const BYTE /*DIRECTION*/ inDirection)
{
#ifdef _DEV_MODE_
	std::cout << "[SEND] 데이터를 전송합니다. 보낼 키값은 : MOVE,  방향은" << (int)inDirection << "\n";
#endif
	PACKET_DATA::CLIENT_TO_MAIN::Move packet(inDirection);
	SendPacket(reinterpret_cast<char*>(&packet));
}

void NetworkManager::SendAttack(const unsigned char inAttackType)
{
	PACKET_DATA::CLIENT_TO_MAIN::Attack packet(inAttackType);
	SendPacket(reinterpret_cast<char*>(&packet));
}

void NetworkManager::SendItem(const unsigned char inItemType)
{
	PACKET_DATA::CLIENT_TO_MAIN::Item packet(inItemType);
	SendPacket(reinterpret_cast<char*>(&packet));
}

void NetworkManager::LogInOrSignUpProcess()
{
	std::cout << "로그인은 1번, 회원가입은 2번을 입력해주세요 : " << std::endl;

	int tempInputtedCommand{};

	std::cin >> tempInputtedCommand;

	switch (tempInputtedCommand)
	{
	case 1:
	{
		std::cout << "ID를 입력해주세요 : " << std::endl;

		WCHAR tempID[9]{};
		wscanf(L"%s", tempID);

		PACKET_DATA::CLIENT_TO_MAIN::Login loginPacket(tempID);
		SendPacket(reinterpret_cast<char*>(&loginPacket));
		break;
	}
	case 2:
	{
		std::cout << "회원가입을 원하시는 ID를 입력해주세요 : " << std::endl;

		WCHAR tempID[9]{};
		wscanf(L"%s", tempID);

		std::cout << "원하시는 직업을 선택해주세요. \n   1. 기사 2. 궁수 3. 마녀" << std::endl;
		int tempInputtedJob{};
		std::cin >> tempInputtedJob;

		if (tempInputtedJob > 0 && tempInputtedJob < 4)
		{
			PACKET_DATA::CLIENT_TO_MAIN::SignUp signUpPacket(tempID, tempInputtedJob);
			SendPacket(reinterpret_cast<char*>(&signUpPacket));
		}
		else
		{
			std::cout << "그런직업 없어요! 잘가요!" << std::endl;
			throw ERROR;
		}
		break;
	}
	}
}
