#include "pch.h"
#include "../Define.h"
#include "QueryDefine.h"

#include "MemoryUnit.h"
#include "SendMemoryPool.h"
#include "GameQueryServer.h"

GameQueryServer::GameQueryServer(bool inNotUse)
	: wsa()
	, hIOCP()
	, socket()
	, serverAddr()
	, workerThreadCont()
	, pRecvMemoryUnit(new MemoryUnit(MEMORY_UNIT_TYPE::RECV))
	, loadedBuf()
	, loadedSize(GLOBAL_DEFINE::MAX_SIZE_OF_RECV_PACKET)
{
	ServerIntegrityCheck();

	SendMemoryPool::MakeInstance();

	PrintServerInfoUI();
	InitNetwork();

	if (InitAndConnectToDB() == false)
	{
		ERROR_HANDLING::ERROR_QUIT(L"DB 초기화 혹은, 접속에 실패하였습니다.");
	}
};

GameQueryServer::~GameQueryServer()
{
	SendMemoryPool::DeleteInstance();
	FreeAndDisconnectDB();

	workerThreadCont.clear();

	delete pRecvMemoryUnit;

	closesocket(socket);
	CloseHandle(hIOCP);
}

void GameQueryServer::ServerIntegrityCheck()
{
	//무결성 검사
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
	printf("■ Listen Port : %d \n", GLOBAL_DEFINE::QUERY_SERVER_PORT );
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
	workerThreadCont.reserve(1);
	for (int i = 0; i < /* (int)si.dwNumberOfProcessors * 2 */ 2; ++i)
	{
		workerThreadCont.emplace_back(std::thread{ StartWorkerThread, (LPVOID)this });
	}

	// 4. 소켓 생성
	if (socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)
		; socket == INVALID_SOCKET) ERROR_QUIT(TEXT("socket()"));

	// 4. 소켓과 입출력 완료 포트 연결
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), hIOCP, socket, 0);

	// 5. 메인 서버 정보 객체 설정
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);	// 추후 MainServer Public IP로 변경해야함!
	serverAddr.sin_port = htons(GLOBAL_DEFINE::MAIN_SERVER_PORT);

	// 6. 커넥트!!
	if (int retVal = connect(socket, (SOCKADDR*)& serverAddr, sizeof(serverAddr))
		; retVal == SOCKET_ERROR) ERROR_QUIT(L"Connect()");

	printf("메인 서버에 정상적으로 연결되었습니다!\n\n");
}

/*
	GameServer::Run()
		- Accept Process 실행 및 Worker Thread join!
*/
void GameQueryServer::Run()
{
	printf("Game Server activated!\n\n");
	for (auto& thread : workerThreadCont) thread.join();
}


/*
	GameServer::StartWorkerThread(LPVOID arg)
		- 쓰레드에서 멤버 변수를 사용하기 위해, 클래스 내부에서 워커쓰레드에 필요한 함수를 호출.
*/
void GameQueryServer::StartWorkerThread(LPVOID arg)
{
	GameQueryServer* pServer = static_cast<GameQueryServer*>(arg);
	pServer->WorkerThreadFunction();
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
	unsigned long long tempKey;

	LPVOID pMemoryUnit;

	while (7)
	{
#pragma region [ Wait For Event ]
		// 입출력 완료 포트에 저장된 결과를 처리하기 위한 함수 // 대기 상태가 됨
		retVal = GetQueuedCompletionStatus(
			hIOCP, //입출력 완료 포트 핸들
			&cbTransferred, //비동기 입출력 작업으로, 전송된 바이트 수가 여기에 저장된다.
			&tempKey, //함수 호출 시 전달한 세번째 인자(32비트) 가 여기에 저장된다.
			reinterpret_cast<LPOVERLAPPED *>(&pMemoryUnit), //Overlapped 구조체의 주소값
			INFINITE // 대기 시간 -> 깨울 까지 무한대
		);
#pragma endregion

#pragma region [ Error Exception ]
		//ERROR_CLIENT_DISCONNECT:
		if (retVal == 0 || cbTransferred == 0)
		{
			assert(false, L"MainServer가 비정상적입니다. 확인이 필요합니다.");
			//NETWORK_UTIL::LogOutProcess(pMemoryUnit);
			continue;
		}
#pragma endregion

		reinterpret_cast<MemoryUnit *>(pMemoryUnit)->memoryUnitType == MEMORY_UNIT_TYPE::RECV
			? AfterRecv(cbTransferred)
			: AfterSend(reinterpret_cast<MemoryUnit*>(pMemoryUnit));
	}
}

/*
	GameServer::AfterRecv(SocketInfo* pClient)
		- 리시브 함수 호출 후, 클라이언트의 데이터를 실제로 받았을 때, 호출되는 함수.
*/
void GameQueryServer::AfterRecv(int cbTransferred)
{
	// 받은 데이터 처리
	ProcessRecvData(cbTransferred);

	// 바로 다시 Recv!
	RecvPacket();
}

/*
	GameServer::ProcessRecvData(SocketInfo* pClient, int restSize)
		- 받은 데이터들을 패킷화하여 처리하는 함수.
*/
void GameQueryServer::ProcessRecvData(int restSize)
{
	char *pBuf = pRecvMemoryUnit->dataBuf; // pBuf -> 처리하는 문자열의 시작 위치
	char packetSize{ 0 }; // 처리해야할 패킷의 크기

	// 이전에 처리를 마치지 못한 버퍼가 있다면, 지금 처리해야할 패킷 사이즈를 알려줘.
	if (0 < loadedSize) packetSize = loadedBuf[0];

	// 처리하지 않은 버퍼의 크기가 있으면, 계속 루프문을 돕니다.
	while (restSize > 0)
	{
		// 이전에 처리를 마치지 못한 버퍼를 처리해야한다면 패스, 아니라면 지금 처리해야할 패킷의 크기를 받음.
		if (packetSize == 0) packetSize = static_cast<int>(pBuf[0]);

		// 처리해야하는 패킷 사이즈 중에서, 이전에 이미 처리한 패킷 사이즈를 빼준다.
		int required = packetSize - loadedSize;

		// 패킷을 완성할 수 있을 때 (요청해야할 사이즈보다, 남은 사이즈가 크거나 같을 때)
		if (restSize >= required)
		{
			memcpy(loadedBuf + loadedSize, pBuf, required);

			//-------------------------------------------------------------------------------
			ProcessPacket(); //== pClient->pZone->ProcessPacket(pClient); // 패킷처리 가가가가아아아아즈즈즈즞즈아아아아앗!!!!!!
			//-------------------------------------------------------------------------------

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
		}
	}
}

/*
	GameServer::AfterSend(SocketInfo* pClient)
		- WSASend 함수 호출 후, 데이터 전송이 끝났을 때, 호출되는 함수.
*/
void GameQueryServer::AfterSend(MemoryUnit* pMemoryUnit)
{
	// 보낼 때 사용한 버퍼 후처리하고 끝! ( 오버랩 초기화는 보낼떄 처리)
	SendMemoryPool::GetInstance()->PushMemory(pMemoryUnit);
}

void GameQueryServer::ProcessPacket()
{
	using namespace PACKET_TYPE;

	switch (loadedBuf[1])
	{
	case MAIN_TO_QUERY::DEMAND_LOGIN:
		ProcessDemandLogin();
		break;
	case MAIN_TO_QUERY::SAVE_LOCATION:
		ProcessSaveLocation();
		break;
	default:
		assert(false, "정의되지 않은 프로토콜을 받았습니다.");
		break;
	}
}

/*
	SendPacket()
		- WSASend!는 여기에서만 존재할 수 있습니다.

	!0. 단순히 WSA Send만 있는 함수입니다. 데이터는 준비해주세요.
	!1. 이 함수를 호출하기 전에, wsaBuf의 len을 설정해주세요.

	?0. wsaBuf의 buf는 보낼때마다 바꿔줘야 할까요?
*/

void GameQueryServer::SendPacket(char* packetData)
{
	MemoryUnit* sendMemoryUnit = SendMemoryPool::GetInstance()->PopMemory();
	memcpy(sendMemoryUnit->dataBuf, packetData, packetData[0]);

	sendMemoryUnit->wsaBuf.len = static_cast<ULONG>(packetData[0]);

#ifdef _DEV_MODE_
	std::cout << "길이 : " << sendMemoryUnit->memoryUnit.wsaBuf.len << "타입 : " << (int)packetData[1] << "내용 : " << (int)packetData[2];
#endif

	ZeroMemory(&(sendMemoryUnit->overlapped), sizeof(sendMemoryUnit->overlapped));

	//ERROR_HANDLING::errorRecvOrSendArr[
	//	static_cast<bool>(
			//1 + 
	if (SOCKET_ERROR ==
		WSASend(socket, &(sendMemoryUnit->wsaBuf), 1, NULL, 0, &(sendMemoryUnit->overlapped), NULL)
		)
	{
		ERROR_HANDLING::ERROR_DISPLAY(L"SendPacket()");
	}
}

/*
	RecvPacket()
		- WSA Recv는 여기서만 존재합니다.

	!0. SocketInfo에 있는 wsaBuf -> buf 에 리시브를 합니다.
	!1. len은 고정된 값을 사용합니다. MAX_SIZE_OF_RECV_BUFFER!
*/
void GameQueryServer::RecvPacket()
{
	// 받은 데이터에 대한 처리가 끝나면 바로 다시 받을 준비.
	DWORD flag{};

	ZeroMemory(&(pRecvMemoryUnit->overlapped), sizeof(pRecvMemoryUnit->overlapped));

	//ERROR_HANDLING::errorRecvOrSendArr[
	//	static_cast<bool>(
			//1 + 
	if (SOCKET_ERROR == WSARecv(socket, &(pRecvMemoryUnit->wsaBuf), 1, NULL, &flag /* NULL*/, &(pRecvMemoryUnit->overlapped), NULL))
	{
		ERROR_HANDLING::HandleRecvOrSendError();
		//ERROR_HANDLING::ERROR_DISPLAY("못받았어요....");
	}
	//		)
	//]();
}

void GameQueryServer::ProcessDemandLogin()
{
	SQLWCHAR tempIdBuffer[10]{};
	SQLLEN tempIDType = SQL_NTS;

	memcpy(tempIdBuffer, loadedBuf, 20);

	SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	retcode = SQLBindParameter(hstmt,
		1,	// Parameter Index
		SQL_PARAM_INPUT, // Parameter Type
		SQL_C_WCHAR, // c dataType
		SQL_WCHAR, // db dataType
		10, // size?
		0, // ?
		tempIdBuffer,
		sizeof(tempIdBuffer),
		&tempIDType	// SQL_NTS
	);
	
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"Exec User_DemandLogin", SQL_NTS);

	SQLINTEGER retPosX{}, retPosY{};
	SQLLEN intLen = SQL_INTEGER;

	retcode = SQLBindCol(hstmt, 1, SQL_INTEGER, &retPosX, intLen, &intLen);
	retcode = SQLBindCol(hstmt, 2, SQL_INTEGER, &retPosY, intLen, &intLen);

	std::cout << "받은 위치는  x : " << retPosX << ", y : " << retPosY << std::endl;

	SQLFreeStmt(hstmt, SQL_DROP);
}

void GameQueryServer::ProcessSaveLocation()
{

}

bool GameQueryServer::InitAndConnectToDB()
{
	// Allocate environment handle  
	// Set the ODBC version environment attribute  
	SQLRETURN retcode{};

	if (retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv)
		; retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		// Allocate connection handle  
		if (retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0)
			; retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) 
		{
			// Set login timeout to 5 seconds  
			if (retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc)
				; retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				// Allocate statement handle  
				if (retcode = SQLConnect(hdbc, (SQLWCHAR*)L"ODBC_2013182027", SQL_NTS, (SQLWCHAR*)L"ACCOUNT_TEST", SQL_NTS, (SQLWCHAR*)L"test1234", SQL_NTS)
					; retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
				{
					std::cout << "SQL CONNECT!!\n";
					return true;
				}
			}
		}
	}

	PrintDBErrorMessage(hstmt, SQL_HANDLE_STMT, retcode);
	return false;
}

void GameQueryServer::FreeAndDisconnectDB()
{
	SQLCancel(hstmt);
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

void GameQueryServer::PrintDBErrorMessage(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	WCHAR wszMessage[1000];
	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}