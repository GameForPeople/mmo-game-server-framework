#include "pch.h"
#include "../Define.h"
#include "ServerDefine.h"

#include "Zone.h"
#include "MemoryUnit.h"

#include "TimerManager.h"
#include "ConnectManager.h"
#include "LuaManager.h"

#include "SendMemoryPool.h"

#include "UserData.h"

#include "ObjectInfo.h"
#include "ClientContUnit.h"

#include "GameServer.h"

#include "BaseMonster.h"
#include "MonsterModelManager.h"

// extern!
namespace NETWORK_UTIL
{
	SOCKET querySocket;
	QueryMemoryUnit* queryMemoryUnit;
}

GameServer::GameServer(bool)
	: wsa()
	, hIOCP()
	, listenSocket()
	, serverAddr()
	, workerThreadCont()
	, zone(/*std::make_unique<Zone>()*/ new Zone())
{
	ServerIntegrityCheck();
	
	ConnectManager::MakeInstance();
	SendMemoryPool::MakeInstance();
	LuaManager::MakeInstance(zone);

	InitNetwork();
	AcceptQueryServer();
	TimerManager::MakeInstance(hIOCP);

	PrintServerInfoUI();
};

GameServer::~GameServer()
{
	SendMemoryPool::DeleteInstance();
	TimerManager::DeleteInstance();
	ConnectManager::DeleteInstance();
	LuaManager::DeleteInstance();

	delete zone;
	workerThreadCont.clear();

	closesocket(listenSocket);
	delete NETWORK_UTIL::queryMemoryUnit;
	closesocket(NETWORK_UTIL::querySocket);
	CloseHandle(hIOCP);
}

#pragma region [Framework]
void GameServer::ServerIntegrityCheck()
{
	// 프로그래밍 구조상 오류 유발 제한
	SocketInfo tempSocketInfo(0);
	assert((unsigned long)(&tempSocketInfo) == (unsigned long)(&(tempSocketInfo.memoryUnit.overlapped)), L"SocketInfo 구조체의 필드에서, MemoryUnit(Overlap)이 최상단에 생성되지 않아 비정상적인 결과를 도출할 수 있습니다. 서버 실행을 거절하였습니다. ");
	
	SendMemoryUnit tempSendMemoryUnit{};
	assert((unsigned long)(&tempSendMemoryUnit) == (unsigned long)(&(tempSendMemoryUnit.memoryUnit.overlapped)), L"SendMemoryUnit 구조체의 필드에서, MemoryUnit(Overlap)이 최상단에 생성되지 않아 비정상적인 결과를 도출할 수 있습니다. 서버 실행을 거절하였습니다. ");

	TimerMemoryHead tempTimerMemoryHead{};
	assert((unsigned long)(&tempTimerMemoryHead) == (unsigned long)(&(tempTimerMemoryHead.memoryUnit.overlapped)), L"TimerMemoryHead 구조체의 필드에서, MemoryUnit(Overlap)이 최상단에 생성되지 않아 비정상적인 결과를 도출할 수 있습니다. 서버 실행을 거절하였습니다. ");

	QueryMemoryUnit tempQueryMemoryUnit{};
	assert((unsigned long)(&tempQueryMemoryUnit) == (unsigned long)(&(tempQueryMemoryUnit.memoryUnit.overlapped)), L"QueryMemoryUnit 구조체의 필드에서, MemoryUnit(Overlap)이 최상단에 생성되지 않아 비정상적인 결과를 도출할 수 있습니다. 서버 실행을 거절하였습니다. ");

	static_assert(GLOBAL_DEFINE::MAX_HEIGHT == GLOBAL_DEFINE::MAX_WIDTH,
		L"MAX_HEIGHT와 MAX_WIDTH가 다르며, 이는 현재로직에서 Sector 계산에서 비정상적인 결과를 도출할 수 있습니다. 서버 실행을 거절하였습니다.");

	static_assert((int)((GLOBAL_DEFINE::MAX_HEIGHT - 1) / GLOBAL_DEFINE::SECTOR_DISTANCE)
		!= (int)((GLOBAL_DEFINE::MAX_HEIGHT + 1) / GLOBAL_DEFINE::SECTOR_DISTANCE),
		L"MAX_HEIGHT(그리고 MAX_WIDTH)는 SECTOR_DISTANCE의 배수가 아닐 경우, 비정상적인 결과를 도출할 수 있습니다. 서버 실행을 거절하였습니다.");
	
	static_assert(GLOBAL_DEFINE::MAX_HEIGHT / GLOBAL_DEFINE::SECTOR_DISTANCE < 128,
		L"현재 구조상 Sector Index의 최대 갯수는 BYTE(4바이트)를 초과할 수 없습니다.");

	// 이제 채팅서버에서 해당 내용을 검사합니다!
	//static_assert(PACKET_TYPE::CLIENT_TO_SERVER::CHAT == PACKET_TYPE::SERVER_TO_CLIENT::CHAT,
	//	"CS::CHAT와 SC::CHAT의 값이 다르며, 이는 클라이언트에 치명적인 오류를 발생시킵니다. 서버 실행을 거절하였습니다.");

	// 자료형 통일 검사
	assert(typeid(_KeyType).name() == typeid(PACKET_DATA::_KeyType).name(),
		L"_KeyType이 Define과 InHeaderDefine에서 서로 다르게 정의되어있습니다. 확인해주세요.");

	assert(typeid(_PosType).name() == typeid(PACKET_DATA::_PosType).name(),
		L"_KeyType이 Define과 InHeaderDefine에서 서로 다르게 정의되어있습니다. 확인해주세요.");

	std::wcout << L"#. Server의 무결성 테스트를 통과하였습니다. \n";
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

		if (auto[isTrueAdd, uniqueKey] = ConnectManager::GetInstance()->GetUniqueKey()
			; isTrueAdd)
		{
			zone->zoneContUnit->clientContArr[uniqueKey]->sock = clientSocket;

			// 소켓과 입출력 완료 포트 연결
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), hIOCP, uniqueKey, 0);

			// 비동기 입출력의 시작.
			NETWORK_UTIL::RecvPacket(zone->zoneContUnit->clientContArr[uniqueKey]);
		}
		else {
			closesocket(clientSocket);
		}
#pragma region [ OLD ACCEPT PROCESS ]
		//if (auto [isTrueAdd, pClient] = zone->TryToEnter()
		//	; isTrueAdd)
		//{
		//	// 소켓과 입출력 완료 포트 연결
		//	CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), hIOCP, pClient->objectInfo->key, 0);

		//	pClient->sock = clientSocket;
		//	
		//	// MemoryUnit 생성자에서 보장함.
		//	//pClient->memoryUnit.wsaBuf.buf = pClient->memoryUnit.dataBuf;
		//	//pClient->memoryUnit.wsaBuf.len = GLOBAL_DEFINE::MAX_SIZE_OF_RECV;

		//	// 클라이언트에게 서버에 접속(Accept) 함을 알림
		//	PACKET_DATA::MAIN_TO_CLIENT::LoginOk loginPacket(pClient->objectInfo->key);
		//	NETWORK_UTIL::SendPacket(pClient, reinterpret_cast<char*>(&loginPacket));

		//	// 자신의 캐릭터를 넣어줌.
		//	PACKET_DATA::MAIN_TO_CLIENT::PutPlayer putPacket( pClient->objectInfo->key, pClient->objectInfo->posX, pClient->objectInfo->posY);
		//	NETWORK_UTIL::SendPacket(pClient, reinterpret_cast<char*>(&putPacket));

		//	std::cout << " [HELLO] 클라이언트 (" << inet_ntoa(clientAddr.sin_addr) << ") 가 접속했습니다. \n";
		//	
		//	// 최초 위치에서 처음 뷰리스트와 섹터 갱신.
		//	pClient->pZone->InitViewAndSector(pClient);

		//	// 비동기 입출력의 시작.
		//	NETWORK_UTIL::RecvPacket(pClient);
		//}
		//else {
		//	closesocket(clientSocket);
		//	//delete pClient;	// if nullptr;
		//}
#pragma endregion
	}
}

/*
	GameServer::AcceptQueryServer()
		- QueryServer 와의 연결을 담당합니다.

		!0. 해당 함수는 Run이 호출되기 전에 반드시 호출되어야 합니다.
*/
void GameServer::AcceptQueryServer()
{
	std::cout << "!. QueryServer를 실행시켜주세요!" << std::endl;

	while (7)
	{
		SOCKET tempSocket{};
		SOCKADDR_IN tempAddr{};
		int addrLength = sizeof(tempAddr);

		if (tempSocket = WSAAccept(listenSocket, (SOCKADDR *)&tempAddr, &addrLength, NULL, NULL)
			; tempSocket == INVALID_SOCKET)
		{
			ERROR_HANDLING::ERROR_QUIT(TEXT("accept()"));
		}

		getpeername(tempSocket, reinterpret_cast<SOCKADDR *>(&tempAddr), &addrLength);

		if (ntohs(tempAddr.sin_port) == GLOBAL_DEFINE::QUERY_SERVER_PORT)
		{
			std::cout << "!. QueryServer의 연결이 성공했습니다." << std::endl;
			NETWORK_UTIL::queryMemoryUnit = new QueryMemoryUnit;// std::make_unique<QueryMemoryUnit>();
			NETWORK_UTIL::querySocket = tempSocket;
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(tempSocket), hIOCP, NETWORK_UTIL::querySocket, 0);
			NETWORK_UTIL::RecvQueryPacket();
			return;
		}
		
		std::cout << "!. QueryServer의 연결에 실패했습니다. 지금 접속한 이상한 포트번호 : " << ntohs(tempAddr.sin_port) << std::endl;
		closesocket(tempSocket);

		Sleep(1000);
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
	unsigned long long Key{};
	
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

#pragma region [ Error Exception ]
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
#pragma endregion

#pragma region [ Process ]
		switch (reinterpret_cast<MemoryUnit*>(pMemoryUnit)->memoryUnitType)
		{
		case MEMORY_UNIT_TYPE::SEND_TO_CLIENT:
			//  send에 대한 에러는 처리하지 않고, 보낼 때 사용한 버퍼 후처리하고 끝! ( 오버랩 초기화는 보낼떄 처리)
			SendMemoryPool::GetInstance()->PushMemory(reinterpret_cast<SendMemoryUnit*>(pMemoryUnit));
			
			break;
		case MEMORY_UNIT_TYPE::TIMER_FUNCTION:
			zone->ProcessTimerUnit(Key);
		
			break;
		case MEMORY_UNIT_TYPE::RECV_FROM_CLIENT:
			if (retVal == 0 || cbTransferred == 0)
			{
				//NETWORK_UTIL::LogOutProcess(pMemoryUnit);
				LogOut(reinterpret_cast<SocketInfo*>(pMemoryUnit), false);
				continue;
			}
			else
			{
				// 받은 데이터 처리
				MakePacketFromRecvData(reinterpret_cast<SocketInfo*>(pMemoryUnit), cbTransferred);
				// 바로 다시 Recv!
				NETWORK_UTIL::RecvPacket(reinterpret_cast<SocketInfo*>(pMemoryUnit));
			}
			break;
		case MEMORY_UNIT_TYPE::RECV_FROM_QUERY:
			if (retVal == 0 || cbTransferred == 0)
			{
				std::cout << "쿼리서버가 종료되었습니다. 이제 이 서버는 무슨 짓을 할지 모릅니다.";
				break;
			}
			MakeQueryPacketFromRecvData(cbTransferred);
			NETWORK_UTIL::RecvQueryPacket();
			
			break;
		default:
			std::cout << "\n[Error Memory Unit Type] : " << "정의되지 않은 메모리 타입 -> " <<
				static_cast<int>(reinterpret_cast<MemoryUnit*>(pMemoryUnit)->memoryUnitType)
				<< std::endl;
			break;
		}
#pragma endregion
	}
}
#pragma endregion

#pragma region [Client to Main]
/*
	GameServer::ProcessRecvData(SocketInfo* pClient, int restSize)
		- 받은 데이터들을 패킷화하여 처리하는 함수.
*/
void GameServer::MakePacketFromRecvData(SocketInfo* pClient, int restSize)
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
			ProcessPacket(pClient); //== pClient->pZone->ProcessPacket(pClient); // 패킷처리 가가가가아아아아즈즈즈즞즈아아아아앗!!!!!!
			//-------------------------------------------------------------------------------

			pClient->loadedSize = 0;
			restSize -= required;
			pBuf += required;
			packetSize = 0;
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
	Zone::ProcessRecvData()
		- 받은 데이터들을 함수와 연결해줍니다.
*/
void GameServer::ProcessPacket(SocketInfo* pClient)
{
	using namespace PACKET_TYPE::CLIENT_TO_MAIN;
	//recvFunctionArr[(pClient->loadedBuf[1]) % (PACKET_TYPE::CLIENT_TO_MAIN::ENUM_SIZE)](*this, pClient);
	switch (pClient->loadedBuf[1])
	{
	case MOVE:
		if (pClient->objectInfo->hp == 0) break;
		if (pClient->objectInfo->moveFlag == false) break;
		zone->RecvCharacterMove(pClient);
		break;
	case LOGIN:
		RecvLogin(pClient);
		break;
	case SIGN_UP:
		RecvSignUp(pClient);
		break;
	case ATTACK:
		RecvAttack(pClient);
		break;
	case USE_ITEM:
		RecvItem(pClient);
		break;
	case CHAT:
		RecvChat(pClient);
		break;
	}
}

void GameServer::RecvLogin(SocketInfo* pClient)
{
	PACKET_DATA::MAIN_TO_QUERY::DemandLogin packet(
		pClient->key,
		pClient->loadedBuf + 2,
		0
	);

	pClient->RegisterNewNickName(packet.id);

	NETWORK_UTIL::SendQueryPacket(reinterpret_cast<char*>(&packet));
}

void GameServer::RecvSignUp(SocketInfo* pClient)
{
	PACKET_DATA::CLIENT_TO_MAIN::SignUp* recvPacket = reinterpret_cast<PACKET_DATA::CLIENT_TO_MAIN::SignUp*>(pClient->loadedBuf);

	PACKET_DATA::MAIN_TO_QUERY::DemandSignUp packet(
		pClient->key,
		pClient->loadedBuf + 2,
		recvPacket->job
	);

	pClient->RegisterNewNickName(packet.id);
	NETWORK_UTIL::SendQueryPacket(reinterpret_cast<char*>(&packet));
}

void GameServer::RecvAttack(SocketInfo* pClient)
{
	if (pClient->objectInfo->hp)
	{
		PACKET_DATA::CLIENT_TO_MAIN::Attack* packet = reinterpret_cast<PACKET_DATA::CLIENT_TO_MAIN::Attack*>(pClient->loadedBuf);

		if (packet->attackType == 0)	// 평타
		{
			if (pClient->objectInfo->attackFlag == false) return; // 공격 아직 못함
			if (pClient->objectInfo->hp == 0) return;	// [NOT_CAS] 여기서 안죽었으면, 떄리는거 정도는 허용하겠다.

			if (ATOMIC_UTIL::T_CAS(&(pClient->objectInfo->attackFlag), true, false))
			{
				auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
				timerUnit->objectKey = pClient->key;
				timerUnit->timerType = TIMER_TYPE::PLAYER_ATTACK;
				TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::SECOND);

				pClient->monsterViewListLock.lock_shared();				// ++++++++++++++++++++++++++++++++++++1
				auto localMonsterviewList = pClient->monsterViewList;
				pClient->monsterViewListLock.unlock_shared();			// ------------------------------------0

				// [NOT_CAS]  이 함수 후 레벨업을 한다면, 레벨업 전 공격 데미지를 적용하겠다.
				const int baseDamage = JOB::GetDamage(pClient->objectInfo->job, pClient->objectInfo->level);

				for (auto iterKey : localMonsterviewList)
				{
					auto pMonster = zone->zoneContUnit->monsterCont[BIT_CONVERTER::WhatIsYourTypeAndRealKey(iterKey).second];

					// [NOT_CAS] 이 복사 후, 몬스터, 캐릭터가 이동한다 해도, 이전 위치값으로 처리하겠다.
					if (JOB::IsAttack(pClient->objectInfo->job, packet->attackType, pClient, pMonster))
					{
						// [NOT_CAS] 이 함수 후, 캐릭터가 상태 변경을 하더라도 상태변경 전 데미지를 적용하겠다.
						int realDamage = baseDamage;
						if (pMonster->electricTick) realDamage *= DAMAGE::ELECTRIC_DAMAGE_NUMBER;

						while (7)
						{
							unsigned short oldHp = pMonster->objectInfo->hp;
							if (oldHp > 0)
							{
								short newHp = oldHp - realDamage;
								if (newHp < 0) newHp = 0;

								// 데미지를 적용하려합니다. 성공하겠습니까?
								if (ATOMIC_UTIL::T_CAS(&(pMonster->objectInfo->hp), oldHp, static_cast<unsigned short>(newHp)))
								{
									if (newHp == 0)
									{
										unsigned int gettedExp = pMonster->objectInfo->level * pMonster->monsterModel->expPerLevel;
										bool isLevelUp{ false };

										while (7)
										{
											unsigned int oldExp = pClient->objectInfo->exp;
											unsigned int newExp = oldExp + gettedExp;

											// [OK_CAS] 이 함수 도중, 캐릭터가 레벨업하면 어짜피 Fail함. 메롱
											if (newExp > JOB::MAX_EXP_PER_LEVEL * pClient->objectInfo->level) newExp = 0;

											// 경험치를 변경하려합니다. 성공하겠습니까?
											if (ATOMIC_UTIL::T_CAS(&(pClient->objectInfo->exp), oldExp, newExp))
											{
												if (newExp == 0)
												{
													if(pClient->objectInfo->level < JOB::MAX_LEVEL) pClient->objectInfo->level.fetch_add(1);	// 이거는 CAS 안써도돼!... 진짜?

													// 이부분에서, hp = 0 일때 문제가 될수 있을듯 한데..이걸 어케처리해야하지..
													pClient->objectInfo->hp = JOB::GetMaxHP(pClient->objectInfo->job, pClient->objectInfo->level);
													pClient->objectInfo->mp = JOB::GetMaxMP(pClient->objectInfo->job, pClient->objectInfo->level);

													NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::HP, pClient->objectInfo->hp, pClient);
													NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::MP, pClient->objectInfo->mp, pClient);

													//재화획득 처리
													switch (pMonster->objectInfo->posY % 6)
													{
													case 0:
													case 1:
													case 2:
														// 너넨 국물도 없어!
														break;
													case 3:
													{
														// 돈줄겡
														pClient->objectInfo->money.fetch_add(pMonster->monsterModel->moneyPerLevel * pMonster->objectInfo->level);
														NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::MONEY, pClient->objectInfo->money, pClient);
														break;
													}
													case 4:
													{
														// 포션줄겡
														pClient->objectInfo->redCount.fetch_add(1);
														NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::RED_P, pClient->objectInfo->redCount, pClient);
														break;
													}
													case 5:
													{
														// 포션줄겡
														pClient->objectInfo->blueCount.fetch_add(1);
														NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::BLUE_P, pClient->objectInfo->blueCount, pClient);
														break;
													}
													}

													auto pTempObjectInfo = pClient->objectInfo;

													PACKET_DATA::MAIN_TO_QUERY::SaveUserInfo packet(
														1,	// 백업용
														static_cast<PlayerObjectInfo*>(pTempObjectInfo)->nickname,
														pTempObjectInfo->posX,
														pTempObjectInfo->posY,
														pTempObjectInfo->level,
														pTempObjectInfo->exp,
														pTempObjectInfo->job,
														pTempObjectInfo->hp,
														pTempObjectInfo->mp,
														pTempObjectInfo->money,
														pTempObjectInfo->redCount,
														pTempObjectInfo->blueCount,
														pTempObjectInfo->treeCount
													);

													NETWORK_UTIL::SendQueryPacket(reinterpret_cast<char*>(&packet));
													isLevelUp = true; // 이 지옥같은 CAS 나가서 레벨업했다고 알려줄꺼야
												}

												NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::EXP, newExp, pClient);
												// 카스 지옥 으악
												break;
											}
										}
										
										// 몬스터 Sector에서 내보내주고. 몬스터 죽었다고 주변에도 알려주고
										zone->DeathForNpc(pMonster);

										auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
										timerUnit->timerType = TIMER_TYPE::REVIVAL;
										timerUnit->objectKey = iterKey;
										TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::MONSTER_REVIVAL);

										NETWORK_UTIL::SEND::SendChatMessage((L"를 사냥하여," + std::to_wstring(gettedExp) + L"의 경험치를 획득했습니다.").c_str(), iterKey, pClient);

										if (isLevelUp) NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::LEVEL, pClient->objectInfo->level, pClient);
									}
									else
									{
										NETWORK_UTIL::SEND::SendChatMessage((L"를 공격하여," + std::to_wstring(realDamage) + L"의 데미지를 입혔습니다.").c_str(), iterKey, pClient);
									}
									break;
								}
							}
							else
							{
								// 이미돌아가셧습니다.
								break;
							}
						}
					}
				}
			}
		}
		else if (packet->attackType == 1)
		{
			if (pClient->objectInfo->skill1Flag == false) return; // 아직 쿨타임
			if (pClient->objectInfo->hp == 0) return;	// [NOT_CAS] 여기서 안죽었으면, 떄리는거 정도는 허용하겠다.

			if (ATOMIC_UTIL::T_CAS(&(pClient->objectInfo->skill1Flag), true, false))
			{
				if (pClient->objectInfo->job == JOB_TYPE::KNIGHT)
				{
					pClient->objectInfo->noDamageFlag = true;
					if (pClient->objectInfo->hp > 0)
					{
						auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
						timerUnit->objectKey = pClient->key;
						timerUnit->timerType = TIMER_TYPE::SKILL_1_COOLTIME;
						TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::KNIGHT_SKILL_1);

						NETWORK_UTIL::SEND::SendChatMessage(L" : 전사스킬 1 [단단해요 국산갑옷] 5초간 무적 발동!", pClient->key, pClient);

						auto timerUnit2 = TimerManager::GetInstance()->PopTimerUnit();
						timerUnit2->objectKey = pClient->key;
						timerUnit2->timerType = TIMER_TYPE::CC_NODAMAGE;
						TimerManager::GetInstance()->AddTimerEvent(timerUnit2, TIME::CC_NODAMAGE);
					}
					else // 데미지가 무적보다 우선순위를 높게 하겠다.
					{
						// 죽은거여 다른거는 몬스터가 처리할거여
						pClient->objectInfo->noDamageFlag = false;
					}
					return; // 저는! 공격스킬 아니에여!
				}
				else if (pClient->objectInfo->job == JOB_TYPE::ARCHER)
				{
					auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
					timerUnit->objectKey = pClient->key;
					timerUnit->timerType = TIMER_TYPE::SKILL_1_COOLTIME;
					TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::ARCHER_SKILL_1);

					NETWORK_UTIL::SEND::SendChatMessage(L" : 궁수스킬 1 [아이스 아메리카노] 피격대상 3초간 동결!", pClient->key, pClient);
				}
				else if (pClient->objectInfo->job == JOB_TYPE::WITCH)
				{
					auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
					timerUnit->objectKey = pClient->key;
					timerUnit->timerType = TIMER_TYPE::SKILL_1_COOLTIME;
					TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::WITCH_SKILL_1);

					NETWORK_UTIL::SEND::SendChatMessage(L" : 마녀스킬 1 [전기 통닭] 피격대상 3초간 감전!", pClient->key, pClient);
				}

				pClient->monsterViewListLock.lock_shared();				// ++++++++++++++++++++++++++++++++++++1
				auto localMonsterviewList = pClient->monsterViewList;
				pClient->monsterViewListLock.unlock_shared();			// ------------------------------------0

				// [NOT_CAS]  이 함수 후 레벨업을 한다면, 레벨업 전 공격 데미지를 적용하겠다.
				const int baseDamage = JOB::GetDamage(pClient->objectInfo->job, pClient->objectInfo->level);

				for (auto iterKey : localMonsterviewList)
				{
					auto pMonster = zone->zoneContUnit->monsterCont[BIT_CONVERTER::WhatIsYourTypeAndRealKey(iterKey).second];

					// [NOT_CAS] 이 복사 후, 몬스터, 캐릭터가 이동한다 해도, 이전 위치값으로 처리하겠다.
					if (JOB::IsAttack(pClient->objectInfo->job, packet->attackType, pClient, pMonster))
					{
						// [NOT_CAS] 이 함수 후, 캐릭터가 상태 변경을 하더라도 상태변경 전 데미지를 적용하겠다.
						int realDamage = baseDamage;
						if (pMonster->electricTick) realDamage *= DAMAGE::ELECTRIC_DAMAGE_NUMBER;

						while (7)
						{
							unsigned short oldHp = pMonster->objectInfo->hp;
							if (oldHp > 0)
							{
								short newHp = oldHp - realDamage;
								if (newHp < 0) newHp = 0;

								// 데미지를 적용하려합니다. 성공하겠습니까?
								if (ATOMIC_UTIL::T_CAS(&(pMonster->objectInfo->hp), oldHp, static_cast<unsigned short>(newHp)))
								{
									if (newHp == 0)
									{
										unsigned int gettedExp = pMonster->objectInfo->level * pMonster->monsterModel->expPerLevel;
										bool isLevelUp{ false };

										while (7)
										{
											unsigned int oldExp = pClient->objectInfo->exp;
											unsigned int newExp = oldExp + gettedExp;

											// [OK_CAS] 이 함수 도중, 캐릭터가 레벨업하면 어짜피 Fail함. 메롱
											if (newExp > JOB::MAX_EXP_PER_LEVEL * pClient->objectInfo->level) newExp = 0;

											// 경험치를 변경하려합니다. 성공하겠습니까?
											if (ATOMIC_UTIL::T_CAS(&(pClient->objectInfo->exp), oldExp, newExp))
											{
												if (newExp == 0)
												{
													if (pClient->objectInfo->level < JOB::MAX_LEVEL) pClient->objectInfo->level.fetch_add(1);	// 이거는 CAS 안써도돼!... 진짜?

													// 이부분에서, hp = 0 일때 문제가 될수 있을듯 한데..이걸 어케처리해야하지..
													pClient->objectInfo->hp = JOB::GetMaxHP(pClient->objectInfo->job, pClient->objectInfo->level);
													pClient->objectInfo->mp = JOB::GetMaxMP(pClient->objectInfo->job, pClient->objectInfo->level);

													NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::HP, pClient->objectInfo->hp, pClient);
													NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::MP, pClient->objectInfo->mp, pClient);

													//재화획득 처리
													switch (pMonster->objectInfo->posY % 6)
													{
													case 0:
													case 1:
													case 2:
														// 너넨 국물도 없어!
														break;
													case 3:
													{
														// 돈줄겡
														pClient->objectInfo->money.fetch_add(pMonster->monsterModel->moneyPerLevel * pMonster->objectInfo->level);
														NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::MONEY, pClient->objectInfo->money, pClient);
														break;
													}
													case 4:
													{
														// 포션줄겡
														pClient->objectInfo->redCount.fetch_add(1);
														NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::RED_P, pClient->objectInfo->redCount, pClient);
														break;
													}
													case 5:
													{
														// 포션줄겡
														pClient->objectInfo->blueCount.fetch_add(1);
														NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::BLUE_P, pClient->objectInfo->blueCount, pClient);
														break;
													}
													}

													auto pTempObjectInfo = pClient->objectInfo;

													PACKET_DATA::MAIN_TO_QUERY::SaveUserInfo packet(
														1,	// 백업용
														static_cast<PlayerObjectInfo*>(pTempObjectInfo)->nickname,
														pTempObjectInfo->posX,
														pTempObjectInfo->posY,
														pTempObjectInfo->level,
														pTempObjectInfo->exp,
														pTempObjectInfo->job,
														pTempObjectInfo->hp,
														pTempObjectInfo->mp,
														pTempObjectInfo->money,
														pTempObjectInfo->redCount,
														pTempObjectInfo->blueCount,
														pTempObjectInfo->treeCount
													);

													NETWORK_UTIL::SendQueryPacket(reinterpret_cast<char*>(&packet));

													isLevelUp = true; // 이 지옥같은 CAS 나가서 레벨업했다고 알려줄꺼야
												}

												NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::EXP, newExp, pClient);

												// 카스 지옥 으악
												break;
											}
										}

										// 몬스터 Sector에서 내보내주고. 몬스터 죽었다고 주변에도 알려주고
										zone->DeathForNpc(pMonster);

										auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
										timerUnit->timerType = TIMER_TYPE::REVIVAL;
										timerUnit->objectKey = iterKey;
										TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::MONSTER_REVIVAL);

										NETWORK_UTIL::SEND::SendChatMessage((L"를 사냥하여," + std::to_wstring(gettedExp) + L"의 경험치를 획득했습니다.").c_str(), iterKey, pClient);

										if (isLevelUp) NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::LEVEL, pClient->objectInfo->level, pClient);
									}
									else
									{
										// 전사는 1번 스킬 공격 스킬 아니에요~ 
										// 안죽었으면 CC를 맞아랏!
										if (pClient->objectInfo->job == JOB_TYPE::ARCHER)
										{
											pMonster->freezeTick.fetch_add(1);
											auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
											timerUnit->timerType = TIMER_TYPE::CC_FREEZE;
											timerUnit->objectKey = iterKey;
											TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::CC_FREEZE);

											NETWORK_UTIL::SEND::SendChatMessage((L"를 공격하여," + std::to_wstring(realDamage) + L"의 데미지와 동결상태로 만들었습니다.").c_str(), iterKey, pClient);
										}
										else if (pClient->objectInfo->job == JOB_TYPE::WITCH)
										{
											pMonster->electricTick.fetch_add(1);
											auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
											timerUnit->timerType = TIMER_TYPE::CC_ELECTRIC;
											timerUnit->objectKey = iterKey;
											TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::CC_ELECTRIC);

											NETWORK_UTIL::SEND::SendChatMessage((L"를 공격하여," + std::to_wstring(realDamage) + L"의 데미지와 감전상태로 만들었습니다.").c_str(), iterKey, pClient);
										}
									}
									break;
								}
							}
							else
							{
								// 이미돌아가셧습니다.
								break;
							}
						}
					}
				}
			}
		}
		else if (packet->attackType == 2)
		{
			if (pClient->objectInfo->skill2Flag == false) return; //아직 쿨타임
			if (pClient->objectInfo->hp == 0) return;	// [NOT_CAS] 여기서 안죽었으면, 떄리는거 정도는 허용하겠다.

			if (ATOMIC_UTIL::T_CAS(&(pClient->objectInfo->skill2Flag), true, false))
			{
				auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
				timerUnit->objectKey = pClient->key;
				timerUnit->timerType = TIMER_TYPE::SKILL_2_COOLTIME;

				if (pClient->objectInfo->job == JOB_TYPE::KNIGHT)
				{
					TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::KNIGHT_SKILL_2);
					NETWORK_UTIL::SEND::SendChatMessage(L" : 전사스킬 2 [단단해요 국산 방패] 피격대상 2초간 기절!", pClient->key, pClient);
				}
				else if (pClient->objectInfo->job == JOB_TYPE::ARCHER)
				{
					TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::ARCHER_SKILL_2);
					NETWORK_UTIL::SEND::SendChatMessage(L" : 궁수스킬 2 [여기 샷 추가요] 3번 연속 공격!", pClient->key, pClient);
				}
				else if (pClient->objectInfo->job == JOB_TYPE::WITCH)
				{
					TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::WITCH_SKILL_2);
					NETWORK_UTIL::SEND::SendChatMessage(L" : 마녀스킬 2 [불닭] 피격대상 5초간 화상!", pClient->key, pClient);
				}

				pClient->monsterViewListLock.lock_shared();				// ++++++++++++++++++++++++++++++++++++1
				auto localMonsterviewList = pClient->monsterViewList;
				pClient->monsterViewListLock.unlock_shared();			// ------------------------------------0

				// [NOT_CAS]  이 함수 후 레벨업을 한다면, 레벨업 전 공격 데미지를 적용하겠다.
				const int baseDamage = JOB::GetDamage(pClient->objectInfo->job, pClient->objectInfo->level);

				for (auto iterKey : localMonsterviewList)
				{
					auto pMonster = zone->zoneContUnit->monsterCont[BIT_CONVERTER::WhatIsYourTypeAndRealKey(iterKey).second];

					// [NOT_CAS] 이 복사 후, 몬스터, 캐릭터가 이동한다 해도, 이전 위치값으로 처리하겠다.
					if (JOB::IsAttack(pClient->objectInfo->job, packet->attackType, pClient, pMonster))
					{
						// [NOT_CAS] 이 함수 후, 몬스터가 상태 변경을 하더라도 상태변경 전 데미지를 적용하겠다.
						int realDamage = baseDamage;
						if (pMonster->electricTick) realDamage *= DAMAGE::ELECTRIC_DAMAGE_NUMBER;

						// 여기는 2렙스킬이며 궁수스킬일 경우 3번 연속 공격 합니다.
						if (pClient->objectInfo->job == JOB_TYPE::ARCHER) realDamage *= 3;

						while (7)
						{
							unsigned short oldHp = pMonster->objectInfo->hp;
							if (oldHp > 0)
							{
								short newHp = oldHp - realDamage;
								if (newHp < 0) newHp = 0;

								// 데미지를 적용하려합니다. 성공하겠습니까?
								if (ATOMIC_UTIL::T_CAS(&(pMonster->objectInfo->hp), oldHp, static_cast<unsigned short>(newHp)))
								{
									if (newHp == 0)
									{
										unsigned int gettedExp = pMonster->objectInfo->level * pMonster->monsterModel->expPerLevel;
										bool isLevelUp{ false };

										while (7)
										{
											unsigned int oldExp = pClient->objectInfo->exp;
											unsigned int newExp = oldExp + gettedExp;

											// [OK_CAS] 이 함수 도중, 캐릭터가 레벨업하면 어짜피 Fail함. 메롱
											if (newExp > JOB::MAX_EXP_PER_LEVEL * pClient->objectInfo->level) newExp = 0;

											// 경험치를 변경하려합니다. 성공하겠습니까?
											if (ATOMIC_UTIL::T_CAS(&(pClient->objectInfo->exp), oldExp, newExp))
											{
												if (newExp == 0)
												{
													if (pClient->objectInfo->level < JOB::MAX_LEVEL) pClient->objectInfo->level.fetch_add(1);	// 이거는 CAS 안써도돼!... 진짜?

													// 이부분에서, hp = 0 일때 문제가 될수 있을듯 한데..이걸 어케처리해야하지..
													pClient->objectInfo->hp = JOB::GetMaxHP(pClient->objectInfo->job, pClient->objectInfo->level);
													pClient->objectInfo->mp = JOB::GetMaxMP(pClient->objectInfo->job, pClient->objectInfo->level);

													NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::HP, pClient->objectInfo->hp, pClient);
													NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::MP, pClient->objectInfo->mp, pClient);

													//재화획득 처리
													switch (pMonster->objectInfo->posY % 6)
													{
													case 0:
													case 1:
													case 2:
														// 너넨 국물도 없어!
														break;
													case 3:
													{
														// 돈줄겡
														pClient->objectInfo->money.fetch_add(pMonster->monsterModel->moneyPerLevel * pMonster->objectInfo->level);
														NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::MONEY, pClient->objectInfo->money, pClient);
														break;
													}
													case 4:
													{
														// 포션줄겡
														pClient->objectInfo->redCount.fetch_add(1);
														NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::RED_P, pClient->objectInfo->redCount, pClient);
														break;
													}
													case 5:
													{
														// 포션줄겡
														pClient->objectInfo->blueCount.fetch_add(1);
														NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::BLUE_P, pClient->objectInfo->blueCount, pClient);
														break;
													}
													}

													auto pTempObjectInfo = pClient->objectInfo;

													PACKET_DATA::MAIN_TO_QUERY::SaveUserInfo packet(
														1,	// 백업용
														static_cast<PlayerObjectInfo*>(pTempObjectInfo)->nickname,
														pTempObjectInfo->posX,
														pTempObjectInfo->posY,
														pTempObjectInfo->level,
														pTempObjectInfo->exp,
														pTempObjectInfo->job,
														pTempObjectInfo->hp,
														pTempObjectInfo->mp,
														pTempObjectInfo->money,
														pTempObjectInfo->redCount,
														pTempObjectInfo->blueCount,
														pTempObjectInfo->treeCount
													);

													NETWORK_UTIL::SendQueryPacket(reinterpret_cast<char*>(&packet));
													isLevelUp = true; // 이 지옥같은 CAS 나가서 레벨업했다고 알려줄꺼야
												}

												NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::EXP, newExp, pClient);
												// 카스 지옥 으악
												break;
											}
										}
										// 몬스터 Sector에서 내보내주고. 나 죽었다고 주변에도 알려주고
										zone->DeathForNpc(pMonster);

										// 마 너 죽었어 임마!
										auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
										timerUnit->timerType = TIMER_TYPE::REVIVAL;
										timerUnit->objectKey = iterKey;
										TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::MONSTER_REVIVAL);

										NETWORK_UTIL::SEND::SendChatMessage((L"를 사냥하여," + std::to_wstring(gettedExp) + L"의 경험치를 획득했습니다.").c_str(), iterKey, pClient);

										if (isLevelUp) NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::LEVEL, pClient->objectInfo->level, pClient);
									}
									else
									{
										if (pClient->objectInfo->job == JOB_TYPE::KNIGHT)
										{
											pMonster->objectInfo->faintTick.fetch_add(1);
											auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
											timerUnit->timerType = TIMER_TYPE::CC_FAINT;
											timerUnit->objectKey = iterKey;
											TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::CC_FAINT);
											NETWORK_UTIL::SEND::SendChatMessage((L"를 공격하여," + std::to_wstring(realDamage) + L"의 데미지와 기절상태로 만들었습니다.").c_str(), iterKey, pClient);
										}
										else if (pClient->objectInfo->job == JOB_TYPE::ARCHER)
										{
											NETWORK_UTIL::SEND::SendChatMessage((L"를 3번 연속 공격하여," + std::to_wstring(realDamage) + L"의 데미지를 입혔습니다.").c_str(), iterKey, pClient);
										}
										else if (pClient->objectInfo->job == JOB_TYPE::WITCH)
										{
											pMonster->objectInfo->burnTick.fetch_add(1);
											auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
											timerUnit->timerType = TIMER_TYPE::CC_BURN_3;
											timerUnit->objectKey = iterKey;
											TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::CC_BURN);

											NETWORK_UTIL::SEND::SendChatMessage((L"를 공격하여," + std::to_wstring(realDamage) + L"의 데미지와 화상상태로 만들었습니다.").c_str(), iterKey, pClient);
										}
									}
									break;
								}
							}
							else
							{
								// 이미돌아가셧습니다.
								break;
							}
						}
					}
				}
			}
		}
	}
}

void GameServer::RecvItem(SocketInfo* pClient)
{
	if (pClient->objectInfo->hp)
	{
		PACKET_DATA::CLIENT_TO_MAIN::Item* packet = reinterpret_cast<PACKET_DATA::CLIENT_TO_MAIN::Item*>(pClient->loadedBuf);

		if (packet->useItemType == 0)
		{
			if (pClient->objectInfo->redCount == 0)
			{
				// 거지에요!
				return;
			}
			else if (pClient->objectInfo->hp == JOB::GetMaxHP(pClient->objectInfo->job, pClient->objectInfo->level))
			{
				// 풀피에요!
				return;
			}
			else
			{
				while (7)
				{
					unsigned int oldRedCount = pClient->objectInfo->redCount;
					unsigned int newRedCount = oldRedCount - 1;

					if (oldRedCount == 0) return; // 님 지금 거지에요~

					if (ATOMIC_UTIL::T_CAS(&(pClient->objectInfo->redCount), oldRedCount, newRedCount))
					{
						NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::RED_P, newRedCount, pClient);

						while (7)
						{
							// 여기 tick Count 오버플로우 될수 있는데 확인하지 않겠음.

							unsigned char oldTickCount = pClient->objectInfo->redTickCount;
							unsigned char newTickCount = oldTickCount + 5;

							if (ATOMIC_UTIL::T_CAS(&(pClient->objectInfo->redTickCount), oldTickCount, newTickCount))
							{
								if (oldTickCount == 0)
								{
									// 타이머에 등록필요함.
									auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
									timerUnit->objectKey = pClient->key;
									timerUnit->timerType = TIMER_TYPE::ITEM_HP;
									TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::ITEM_HP);
								}
								// 아니면 원래 타이머가 알아서 해줄겨.
								break;
							}
						}
						break;
					}
				}
			}
		}
		else if (packet->useItemType == 1)
		{
			if (pClient->objectInfo->blueCount == 0)
			{
				// 거지에요!
				return;
			}
			else if (pClient->objectInfo->mp == JOB::GetMaxMP(pClient->objectInfo->job, pClient->objectInfo->level))
			{
				// 풀마나에요!
				return;
			}
			else
			{
				while (7)
				{
					unsigned int oldBlueCount = pClient->objectInfo->blueCount;
					unsigned int newBlueCount = oldBlueCount - 1;

					if (oldBlueCount == 0) return; // 님 지금 거지에요~

					if (ATOMIC_UTIL::T_CAS(&(pClient->objectInfo->blueCount), oldBlueCount, newBlueCount))
					{
						NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::BLUE_P, newBlueCount, pClient);

						while (7)
						{
							// 여기 tick Count 오버플로우 될수 있는데 확인하지 않겠음.

							unsigned char oldTickCount = pClient->objectInfo->blueTickCount;
							unsigned char newTickCount = oldTickCount + 5;

							if (ATOMIC_UTIL::T_CAS(&(pClient->objectInfo->blueTickCount), oldTickCount, newTickCount))
							{
								if (oldTickCount == 0)
								{
									// 타이머에 등록필요함.
									auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
									timerUnit->objectKey = pClient->key;
									timerUnit->timerType = TIMER_TYPE::ITEM_MP;
									TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::ITEM_MP);
								}
								// 아니면 원래 타이머가 알아서 해줄겨.
								break;
							}
						}
						break;
					}
				}
			}
		}
	}
}

void GameServer::RecvChat(SocketInfo* pClient)
{
	PACKET_DATA::CLIENT_TO_MAIN::Chat* packet = reinterpret_cast<PACKET_DATA::CLIENT_TO_MAIN::Chat*>(pClient->loadedBuf);
	
	pClient->viewListLock.lock_shared();
	auto localViewList = pClient->viewList;
	pClient->viewListLock.unlock_shared();

	for (auto iterKey : localViewList)
	{
		NETWORK_UTIL::SEND::SendChatMessage(packet->message, pClient->key, zone->zoneContUnit->clientContArr[iterKey]);
	}
}

#pragma endregion

#pragma region [QUERY]
void GameServer::MakeQueryPacketFromRecvData(int restSize)
{
	char *pBuf = NETWORK_UTIL::queryMemoryUnit->memoryUnit.dataBuf; // pBuf -> 처리하는 문자열의 시작 위치
	char packetSize{ 0 }; // 처리해야할 패킷의 크기

	// 이전에 처리를 마치지 못한 버퍼가 있다면, 지금 처리해야할 패킷 사이즈를 알려줘.
	if (0 < NETWORK_UTIL::queryMemoryUnit->loadedSize) packetSize = NETWORK_UTIL::queryMemoryUnit->loadedBuf[0];

	// 처리하지 않은 버퍼의 크기가 있으면, 계속 루프문을 돕니다.
	while (restSize > 0)
	{
		// 이전에 처리를 마치지 못한 버퍼를 처리해야한다면 패스, 아니라면 지금 처리해야할 패킷의 크기를 받음.
		if (packetSize == 0) packetSize = static_cast<int>(pBuf[0]);

		// 처리해야하는 패킷 사이즈 중에서, 이전에 이미 처리한 패킷 사이즈를 빼준다.
		int required = packetSize - NETWORK_UTIL::queryMemoryUnit->loadedSize;

		// 패킷을 완성할 수 있을 때 (요청해야할 사이즈보다, 남은 사이즈가 크거나 같을 때)
		if (restSize >= required)
		{
			memcpy(NETWORK_UTIL::queryMemoryUnit->loadedBuf + NETWORK_UTIL::queryMemoryUnit->loadedSize, pBuf, required);

			//-------------------------------------------------------------------------------
			ProcessQueryPacket(); //== pClient->pZone->ProcessPacket(pClient); // 패킷처리 가가가가아아아아즈즈즈즞즈아아아아앗!!!!!!
			//-------------------------------------------------------------------------------

			NETWORK_UTIL::queryMemoryUnit->loadedSize = 0;
			restSize -= required;
			pBuf += required;
			packetSize = 0;
		}
		// 패킷을 완성할 수 없을 때
		else
		{
			memcpy(NETWORK_UTIL::queryMemoryUnit->loadedBuf + NETWORK_UTIL::queryMemoryUnit->loadedSize, pBuf, restSize);
			NETWORK_UTIL::queryMemoryUnit->loadedSize += restSize;
			break;
			//restSize = 0; 
		}
	}
}

void GameServer::ProcessQueryPacket()
{
	using namespace PACKET_TYPE;
	switch (NETWORK_UTIL::queryMemoryUnit->loadedBuf[1])
	{
	case QUERY_TO_MAIN::LOGIN_TRUE:
		RecvLoginTrue();
		break;
	case QUERY_TO_MAIN::LOGIN_FALSE:
		RecvLoginFalse();
		break;
	case QUERY_TO_MAIN::LOGIN_ALREADY:
		RecvLoginAlready();
		break;
	case QUERY_TO_MAIN::LOGIN_NEW:
		RecvLoginNew();
		break;
	default:
		assert(false, "정의되지 않은 Query Packet을 받았습니다. \n");
		break;
	}
}

void GameServer::RecvLoginTrue()
{
	PACKET_DATA::QUERY_TO_MAIN::LoginTrue* packet = reinterpret_cast<PACKET_DATA::QUERY_TO_MAIN::LoginTrue*>(NETWORK_UTIL::queryMemoryUnit->loadedBuf);
	SocketInfo* tempSocketInfo = zone->zoneContUnit->clientContArr[packet->key];
	
	tempSocketInfo->SetNewObjectInfo(packet->xPos, packet->yPos, packet->level, packet->exp, packet->job, packet->hp, packet->mp
		, packet->money, packet->redCount, packet->blueCount, packet->treeCount);

	// 클라이언트에게 서버에 접속(Accept) 함을 알림	
	auto tempObjectInfo = tempSocketInfo->objectInfo;
	PACKET_DATA::MAIN_TO_CLIENT::LoginOk loginPacket(tempSocketInfo->key, tempObjectInfo->posX, tempObjectInfo->posY, tempObjectInfo->level,
		tempObjectInfo->exp, tempObjectInfo->job, tempObjectInfo->hp, tempObjectInfo->mp, tempObjectInfo->money, tempObjectInfo->redCount, tempObjectInfo->blueCount, tempObjectInfo->treeCount);
	
	NETWORK_UTIL::SendPacket(tempSocketInfo, reinterpret_cast<char*>(&loginPacket));

	// 자신의 캐릭터를 넣어줌. -> LoginOK에 통합.
	//PACKET_DATA::MAIN_TO_CLIENT::PutPlayer putPacket(pClient->objectInfo->key, pClient->objectInfo->posX, pClient->objectInfo->posY);
	//NETWORK_UTIL::SendPacket(pClient, reinterpret_cast<char*>(&putPacket));

#ifdef _DEV_MODE_
	std::cout << " [HELLO] 클라이언트 (" << packet->key /*inet_ntoa(clientAddr.sin_addr)*/ << ") 가 접속했습니다. \n";
#endif

	// 해당 존에 입장!
	zone->Enter(tempSocketInfo);

	// 비동기 입출력의 시작.
	NETWORK_UTIL::RecvPacket(tempSocketInfo);
}

void GameServer::RecvLoginFalse()
{
	PACKET_DATA::QUERY_TO_MAIN::LoginFail* packet = reinterpret_cast<PACKET_DATA::QUERY_TO_MAIN::LoginFail*>(NETWORK_UTIL::queryMemoryUnit->loadedBuf);
	SocketInfo* tempSocketInfo = zone->zoneContUnit->clientContArr[packet->key];

	// 클라이언트에게 서버에 접속(Accept) 실패함을 알림
	PACKET_DATA::MAIN_TO_CLIENT::LoginFail loginPacket(packet->failReason);

	NETWORK_UTIL::SendPacket(tempSocketInfo, reinterpret_cast<char*>(&loginPacket));

	NETWORK_UTIL::RecvPacket(tempSocketInfo);
}

void GameServer::RecvLoginNew()
{
	PACKET_DATA::QUERY_TO_MAIN::LoginNew* packet = reinterpret_cast<PACKET_DATA::QUERY_TO_MAIN::LoginNew*>(NETWORK_UTIL::queryMemoryUnit->loadedBuf);
	SocketInfo* tempSocketInfo = zone->zoneContUnit->clientContArr[packet->key];

	//_PosType tempPosX;
	//_PosType tempPosY;
	//
	//do
	//{
	//	tempPosX = rand() % GLOBAL_DEFINE::MAX_WIDTH;
	//	tempPosY = rand() % GLOBAL_DEFINE::MAX_HEIGHT;
	//} while (zone->GetMapData()[tempPosY][tempPosX] == false);
	
	tempSocketInfo->SetNewObjectInfo(
		GLOBAL_DEFINE::START_POSITION_X,  //tempPosX, //GLOBAL_DEFINE::START_POSITION_X, 
		GLOBAL_DEFINE::START_POSITION_Y,  //tempPosY, //GLOBAL_DEFINE::START_POSITION_Y,
		1, 
		0, 
		packet->job, 
		JOB::BASE_HP, 
		JOB::BASE_MP, 
		0, 0, 0, 0);

	// 클라이언트에게 서버에 접속(Accept) 함을 알림
	auto tempObjectInfo = tempSocketInfo->objectInfo;
	PACKET_DATA::MAIN_TO_CLIENT::LoginOk loginPacket(tempSocketInfo->key, tempObjectInfo->posX, tempObjectInfo->posY, tempObjectInfo->level,
		tempObjectInfo->exp, tempObjectInfo->job, tempObjectInfo->hp, tempObjectInfo->mp, tempObjectInfo->money, tempObjectInfo->redCount, tempObjectInfo->blueCount, tempObjectInfo->treeCount);

	NETWORK_UTIL::SendPacket(tempSocketInfo, reinterpret_cast<char*>(&loginPacket));

	std::cout << " [HELLO] 클라이언트 (" << packet->key /*inet_ntoa(clientAddr.sin_addr)*/ << ") 가 접속했습니다. \n";

	// 해당 존에 입장!
	zone->Enter(tempSocketInfo);

	NETWORK_UTIL::RecvPacket(tempSocketInfo);
}

void GameServer::RecvLoginAlready()
{
	PACKET_DATA::QUERY_TO_MAIN::LoginAlready* packet = reinterpret_cast<PACKET_DATA::QUERY_TO_MAIN::LoginAlready*>(NETWORK_UTIL::queryMemoryUnit->loadedBuf);
	SocketInfo* tempSocketInfo = zone->zoneContUnit->clientContArr[packet->key];
	SocketInfo* oldSocketInfo = zone->zoneContUnit->clientContArr[packet->oldKey];

	// 지금이거어짜피안불림!!
	//tempSocketInfo->CopyOtherObjectInfo(oldSocketInfo->objectInfo);
	LogOut(oldSocketInfo, true);

	// 클라이언트에게 서버에 접속(Accept) 함을 알림
	auto tempObjectInfo = tempSocketInfo->objectInfo;
	PACKET_DATA::MAIN_TO_CLIENT::LoginOk loginPacket(tempSocketInfo->key, tempObjectInfo->posX, tempObjectInfo->posY, tempObjectInfo->level,
		tempObjectInfo->exp, tempObjectInfo->job, tempObjectInfo->hp, tempObjectInfo->mp, tempObjectInfo->money, tempObjectInfo->redCount, tempObjectInfo->blueCount, tempObjectInfo->treeCount);

	// 해당 존에 입장!
	zone->Enter(tempSocketInfo);

	NETWORK_UTIL::SendPacket(tempSocketInfo, reinterpret_cast<char*>(&loginPacket));
	NETWORK_UTIL::RecvPacket(tempSocketInfo);
}
#pragma endregion

void GameServer::LogOut(SocketInfo* pOutClient, const bool isForced)
{
#ifdef _DEV_MODE_
	{
	SOCKADDR_IN clientAddr;
	int addrLength = sizeof(clientAddr);
	
	getpeername(pOutClient->sock, (SOCKADDR*)& clientAddr, &addrLength);
	std::cout << " [GOODBYE] 클라이언트 (" << inet_ntoa(clientAddr.sin_addr) << ") 가 종료했습니다. \n";
	}
#endif

	if (isForced)
	{
		zone->Death(pOutClient);
		pOutClient->TerminateClient();
	}
	else 
	{
		zone->Death(pOutClient);
		pOutClient->TerminateClient();

		auto pTempObjectInfo = pOutClient->objectInfo;

		PACKET_DATA::MAIN_TO_QUERY::SaveUserInfo packet(
			0,	// 로그아웃용
			static_cast<PlayerObjectInfo*>(pTempObjectInfo)->nickname,
			pTempObjectInfo->posX,
			pTempObjectInfo->posY,
			pTempObjectInfo->level,
			pTempObjectInfo->exp,
			pTempObjectInfo->job,
			pTempObjectInfo->hp,
			pTempObjectInfo->mp,
			pTempObjectInfo->money,
			pTempObjectInfo->redCount,
			pTempObjectInfo->blueCount,
			pTempObjectInfo->treeCount
		);

		NETWORK_UTIL::SendQueryPacket(reinterpret_cast<char*>(&packet));
	}

	closesocket(pOutClient->sock);
	
	auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
	timerUnit->timerType = TIMER_TYPE::PUSH_OLD_KEY;
	timerUnit->objectKey = pOutClient->key;
	TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::MAX_TIME);
}

#pragma region [Legacy Code]
///*
//	GameServer::AfterRecv(SocketInfo* pClient)
//		- 리시브 함수 호출 후, 클라이언트의 데이터를 실제로 받았을 때, 호출되는 함수.
//*/
//void GameServer::AfterRecv(SocketInfo* pClient, int cbTransferred)
//{
//	// 받은 데이터 처리
//	ProcessRecvData(pClient, cbTransferred);
//
//	// 바로 다시 Recv!
//	NETWORK_UTIL::RecvPacket(pClient);
//}
//
///*
//	GameServer::AfterSend(SocketInfo* pClient)
//		- WSASend 함수 호출 후, 데이터 전송이 끝났을 때, 호출되는 함수.
//*/
//void GameServer::AfterSend(SendMemoryUnit* pMemoryUnit)
//{
//	// 보낼 때 사용한 버퍼 후처리하고 끝! ( 오버랩 초기화는 보낼떄 처리)
//	SendMemoryPool::GetInstance()->PushMemory(pMemoryUnit);
//}
//
//void GameServer::ProcessTimerUnit(const int timerManagerContIndex)
//{
//	zone->ProcessTimerUnit(timerManagerContIndex);
//
//	// 재활용하고 싶은건 재활용하고, 반납할건 반납하게하기 위해, 내부에서 정하도록 변경함.
//	//TimerManager::GetInstance()->PushTimerUnit(pUnit);
//}
#pragma endregion

#ifndef DISABLED_UNALLOCATED_MEMORY_SEND
void GameServer::AfterUnallocatedSend(UnallocatedMemoryUnit* pUnit)
{
	SendMemoryPool::GetInstance()->PushUnallocatedMemory(pUnit);
}
#endif
