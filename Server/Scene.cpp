#include "pch.h"
#include "Define.h"

#include "SocketInfo.h"
#include "MoveManager.h"

#include "Scene.h"

Scene::Scene()
	: moveManager()
	, recvFunctionArr()
	, clientArr()
{
	InitManagers();
	InitClientCont();
	InitFunctions();
}

/*
	GameServer::InitManagers()
		- GamsServer의 생성자에서 호출되며, 각 매니저들의 초기화를 담당합니다.
*/
void Scene::ProcessRecvData(SocketInfo* pClient)
{
	recvFunctionArr[(pClient->buf[0]) % (PACKET_TYPE::ENUM_SIZE)](*this, pClient);
}

/*
	Scene::InitManagers()
		- GamsServer의 생성자에서 호출되며, 각 매니저들의 초기화를 담당합니다.
*/
void Scene::InitManagers()
{
	moveManager = std::make_unique<MoveManager>();
}


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
	pClient->buf[0] = MakeSendPacket(PACKET_TYPE::MOVE);
}
