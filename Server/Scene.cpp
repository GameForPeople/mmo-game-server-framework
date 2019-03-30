#include "pch.h"
#include "Define.h"
#include "ServerDefine.h"

#include "MoveManager.h"
#include "UserData.h"

#include "MemoryUnit.h"
#include "SendMemoryPool.h"

#include "Scene.h"

Scene::Scene() : 
	moveManager(nullptr),
	recvFunctionArr(),
	clientCont()
{
	InitManagers();
	InitClientCont();
	InitFunctions();
}

Scene::~Scene()
{
	delete[] recvFunctionArr;
}

/*
	Scene::InitManagers()
		- GamsServer의 생성자에서 호출되며, 각 매니저들의 초기화를 담당합니다.
*/
void Scene::InitManagers()
{
	moveManager = std::make_unique<MoveManager>();
}

/*
	Scene::InitClientCont()
		- GamsServer의 생성자에서 호출되며, 클라이언트 컨테이너들을 초기화합니다.
*/
void Scene::InitClientCont()
{
	clientCont.reserve(GLOBAL_DEFINE::MAX_CLIENT);
	for (int i = 0; i < GLOBAL_DEFINE::MAX_CLIENT; ++i)
	{
		clientCont.emplace_back(false, nullptr);
	}
}

/*
	Scene::InitFunctions()
		- GamsServer의 생성자에서 호출되며, 게임과 관련된 데이터들의 초기화를 담당합니다.
*/
void Scene::InitFunctions()
{
	recvFunctionArr = new std::function<void(Scene&, SocketInfo*)>[PACKET_TYPE::ENUM_SIZE];
	recvFunctionArr[PACKET_TYPE::MOVE] = &Scene::RecvCharacterMove;
}

/*
	Scene::ProcessRecvData()
		- 받은 데이터들을 함수와 연결해줍니다.
*/
void Scene::ProcessPacket(SocketInfo* pClient)
{
	recvFunctionArr[(pClient->loadedBuf[1]) % (PACKET_TYPE::ENUM_SIZE)](*this, pClient);
}

/*
	Scene::InNewClient()
		- 새로운 클라이언트가 접속했을 떄, 이를 컨테이너에 넣어줍니다.

	#!?0. 하나의 물리 서버에서 하나의 씐을 가질 경우, 지금처럼하는게 맞음.
	#!?1. 다만 하나의 서버에서 여러 씐을 가질 경우, 애초에 SocketInfo를 갖고 있고, InNewCliet에 인자로 넣어주는 게맞음.
*/
std::pair<bool, SocketInfo*> Scene::InNewClient()
{
	//std::lock_guard<std::mutex> localLock(addLock);
	addLock.lock();

	for (int index = 0; index < clientCont.size(); ++index)
	{
		if (clientCont[index].first == false)
		{
			clientCont[index].first = true;

			addLock.unlock();

			// 소켓 정보 구조체 할당
			SocketInfo* pClient = new SocketInfo;
			if (pClient == nullptr)
			{
				ERROR_HANDLING::ERROR_QUIT(TEXT("Make_SocketInfo()"));
				break;
			}

			clientCont[index].second = pClient;
			pClient->clientContIndex = index;
			pClient->pScene = this;
			return std::make_pair(true, pClient);
		}
	}
	addLock.unlock();
	return std::make_pair(false, nullptr);
}

/*
	Scene::OutClient()
		- 클라이언트가 다른 레벨로 이동하거나, 로그아웃 될때, 해당 클라이언트를 레벨에서 빼줍니다.

	#!?0. 도대체 여기서 어디까지 보장을 해줘야하는건지. 이 보장이 오히려 버그가 될 수 있지 않은지.
*/
void Scene::OutClient(SocketInfo* pClient)
{
	// 굳이 Lock 걸 필요 없음.
	clientCont[pClient->clientContIndex].first = false;
	// second는 초기화 할 필요 없음.

	// 다만 이부분에서, 비용이 조금 더 나가더라도, 안정성을 보장하기 위해 처리해주도록 합시다.

	// 사실 pScene을 nullptr하고, LogOutProcess에서, 해당 여부만 검사하는 것도 날 듯 한데;
	pClient->pScene = nullptr;
	pClient->clientContIndex = -1;
}


/*
	Scene::RecvCharacterMove(SocketInfo* pClient)
		- 클라이언트로부터 CharacterMove를 받았을 경우, 호출되는 함수.

	#0. 해당 클라이언트의 Move를 먼저 처리합니다.
	#1. Scene에 존재하는 자신을 포함한 모든 클라이언트에게 ID값과 위치값을 전송합니다.
*/
void Scene::RecvCharacterMove(SocketInfo* pClient)
{
#ifdef _DEV_MODE_
	std::cout << "[AfterRecv] 받은 버퍼는" << int(pClient->loadedBuf[0]) << "희망하는 방향은 : " << int(pClient->loadedBuf[1]) << "\n";
#endif
	moveManager->MoveCharacter(pClient);
	//moveManager->SendMoveCharacter(pClient);
	const BYTE clientPositionByte 
		= BIT_CONVERTER::MakeByteFromLeftAndRightByte(pClient->userData->GetPosition().x, pClient->userData->GetPosition().y);

	for (std::pair<bool, SocketInfo*> retClient : clientCont)
	{
		if (retClient.first) 
		{
			SendMemoryUnit* sendMemoryUnit = SendMemoryPool::GetInstance()->PopMemory(retClient.second);

			sendMemoryUnit->memoryUnit.dataBuf[0] = PACKET_TYPE::MOVE;
			sendMemoryUnit->memoryUnit.dataBuf[1] = pClient->clientContIndex;
			sendMemoryUnit->memoryUnit.dataBuf[2] = clientPositionByte;

			sendMemoryUnit->memoryUnit.wsaBuf.len = 3;

			NETWORK_UTIL::SendPacket(pClient, sendMemoryUnit);
		}
	}
}
