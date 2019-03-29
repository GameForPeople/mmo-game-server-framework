#include "pch.h"
#include "Define.h"
#include "ServerDefine.h"

#include "MoveManager.h"
#include "SocketInfo.h"

#include "Scene.h"

Scene::Scene() : 
	//moveManager(nullptr),
	recvFunctionArr(),
	clientArr()
{
	InitManagers();
	InitClientCont();
	InitFunctions();
}

Scene::~Scene()
{

}

/*
	Scene::ProcessRecvData()
		- 받은 데이터들의 함수와 연결해줍니다.
*/
void Scene::ProcessRecvData(SocketInfo* pClient)
{
	recvFunctionArr[(pClient->buf[0]) % (PACKET_TYPE::ENUM_SIZE)](*this, pClient);
}

/*
	Scene::AddNewClient()
		- 새로운 클라이언트가 접속했을 떄, 이를 컨테이너에 넣어줍니다.
*/
bool Scene::AddNewClient(SocketInfo* pClient)
{
	//std::lock_guard<std::mutex> localLock(addLock);
	addLock.lock();

	for (int index = 0; index < clientArr.size(); ++index)
	{
		if (clientArr[index].first == false)
		{
			clientArr[index].first = true;

			addLock.unlock();

			clientArr[index].second = pClient;
			pClient->clientContIndex = index;
			return true;
		}
	}
	addLock.unlock();
	return false;
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
	for (auto& client : clientArr)
	{
		client.first = false;
		client.second = nullptr;
	}
}

/*
	Scene::InitFunctions()
		- GamsServer의 생성자에서 호출되며, 게임과 관련된 데이터들의 초기화를 담당합니다.
*/
void Scene::InitFunctions()
{
	recvFunctionArr[PACKET_TYPE::MOVE] = &Scene::RecvCharacterMove;
}

/*
	Scene::RecvCharacterMove(SocketInfo* pClient)
		- 클라이언트로부터 CharacterMove를 받았을 경우, 호출되는 함수.
*/
void Scene::RecvCharacterMove(SocketInfo* pClient)
{
#ifdef _DEV_MODE_
	std::cout << "[AfterRecv] 받은 버퍼는" << int(pClient->buf[0]) << "희망하는 방향은 : " << int(pClient->buf[1]) << "\n";
#endif
	moveManager->MoveCharacter(pClient);

	moveManager->SendMoveCharacter(pClient);
	pClient->buf[0] = GLOBAL_UTIL::BIT_CONVERTER::MakeSendPacket(PACKET_TYPE::MOVE);
}
