#include "pch.h"
#include "Define.h"
#include "ServerDefine.h"

#include "ConnectManager.h"
#include "MoveManager.h"

#include "ClientContUnit.h"

#include "UserData.h"

#include "MemoryUnit.h"
#include "SendMemoryPool.h"

#include "Sector.h"

#include "Scene.h"

Scene::Scene() : 
	moveManager(nullptr),
	connectManager(nullptr),
	sceneContUnit(nullptr),
	recvFunctionArr()
{
	InitManagers();
	InitClientCont();
	InitSector();
	InitFunctions();
}

Scene::~Scene()
{
	delete[] recvFunctionArr;
	delete[] sceneContUnit;
}

/*
	Scene::InitManagers()
		- GamsServer의 생성자에서 호출되며, 각 매니저들의 초기화를 담당합니다.
*/
void Scene::InitManagers()
{
	moveManager = std::make_unique<MoveManager>();
	connectManager = std::make_unique<ConnectManager>();
}

/*
	Scene::InitClientCont()
		- GamsServer의 생성자에서 호출되며, 클라이언트 컨테이너들을 초기화합니다.
*/
void Scene::InitClientCont()
{
	sceneContUnit = new SceneContUnit;
	sceneContUnit->clientCont.reserve(GLOBAL_DEFINE::MAX_CLIENT);

	for (int i = 0; i < GLOBAL_DEFINE::MAX_CLIENT; ++i)
	{
		sceneContUnit->clientCont.emplace_back(false, nullptr);
	}
}

/*
	Scene::InitFunctions()
		- GamsServer의 생성자에서 호출되며, 게임과 관련된 데이터들의 초기화를 담당합니다.
*/
void Scene::InitFunctions()
{
	recvFunctionArr = new std::function<void(Scene&, SocketInfo*)>[PACKET_TYPE::CS::ENUM_SIZE];
	recvFunctionArr[PACKET_TYPE::CS::LEFT] = &Scene::RecvCharacterMove;
	recvFunctionArr[PACKET_TYPE::CS::UP] = &Scene::RecvCharacterMove;
	recvFunctionArr[PACKET_TYPE::CS::RIGHT] = &Scene::RecvCharacterMove;
	recvFunctionArr[PACKET_TYPE::CS::DOWN] = &Scene::RecvCharacterMove;
}

/*
	Scene::InitSectorCont()
		- GamsServer의 생성자에서 호출되며, 섹터들을 초기화합니다.
*/
void Scene::InitSector()
{
	// 상수 무결성 검사
	if(GLOBAL_DEFINE::MAX_HEIGHT != GLOBAL_DEFINE::MAX_WIDTH) std::cout << "### Scene의 상수가 비정상적입니다. 확인해주세요.";
	if((int)((GLOBAL_DEFINE::MAX_HEIGHT - 1) / GLOBAL_DEFINE::SECTOR_DISTANCE) 
		== (int)((GLOBAL_DEFINE::MAX_HEIGHT + 1) / GLOBAL_DEFINE::SECTOR_DISTANCE)) std::cout << "### Scene의 상수가 비정상적입니다. 확인해주세요.";

	constexpr BYTE sectorContCount = GLOBAL_DEFINE::MAX_HEIGHT / GLOBAL_DEFINE::SECTOR_DISTANCE;
	
	//X축에 대한, Sector 생성
	sectorCont.reserve(sectorContCount);
	for (int i = 0; i < sectorContCount; ++i)
	{
		sectorCont.emplace_back();
	}

	//Y축에 대한, Sector 생성
	for (int i = 0; i < sectorContCount; ++i)
	{
		sectorCont[i].reserve(sectorContCount);

		for (int j = 0; j < sectorContCount; ++j)
		{
			sectorCont[i].emplace_back(j, i);
		}
	}
}

/*
	Scene::ProcessRecvData()
		- 받은 데이터들을 함수와 연결해줍니다.
*/
void Scene::ProcessPacket(SocketInfo* pClient)
{
	recvFunctionArr[(pClient->loadedBuf[1]) % (PACKET_TYPE::CS::ENUM_SIZE)](*this, pClient);
}

/*
	Scene::InNewClient()
		- connectManager에서 해당 함수 처리하도록 변경.
*/
/*std::optional<SocketInfo*>*/ 
_ClientNode /* == std::pair<bool, SocketInfo*>*/ Scene::InNewClient()
{
	if (auto retNode = connectManager->InNewClient(sceneContUnit, this)
		; retNode.first)
	{
		sectorCont[5][5].InNewClient(retNode.second);
	}
	else return retNode;
}

/*
	Scene::OutClient()
		- connectManager에서 해당 함수 처리하도록 변경.
*/
void Scene::OutClient(SocketInfo* pOutClient)
{
	sectorCont[pOutClient->sectorIndexY][pOutClient->sectorIndexX].OutClient(pOutClient);
	connectManager->OutClient(pOutClient, sceneContUnit);
}

/*
	FindPossibleSectors? CheckPossibleSectors?
		- 현재 캐릭터의 섹터와 위치를 검사하여, 시야 체크를 해야하는 섹터를 검사합니다.

	?0. 기존의 지역변수를 생성하여, 리턴하는 방식에서, SocketInfo의 멤버변수를 두는 방식으로 변경하는게 날꺼 같은디?
*/

void Scene::RenewPossibleSectors(SocketInfo* pClient)
{
	// 로컬 변수를 리턴하는 코드에서, 멤버 변수를 변경하는 방식으로 변경.
	//std::vector<std::pair<BYTE, BYTE>> retVector;
	//retVector.reserve(4);	// Max Possible Sector!

	//retVector.emplace_back(std::make_pair(pClient->sectorIndexX, pClient->sectorIndexY));

	const BYTE tempX = sectorCont[pClient->sectorIndexX][pClient->sectorIndexY].GetCenterX();
	const BYTE tempY = sectorCont[pClient->sectorIndexX][pClient->sectorIndexY].GetCenterY();

	UserData* pTempUserData = pClient->userData;

	char xDir = 0;
	char yDir = 0;

	if (pTempUserData->GetPosition().x > tempX) { xDir = 1; }
	else if (pTempUserData->GetPosition().x < tempX - 1) { xDir = -1; }

	if (pTempUserData->GetPosition().y > tempY) { yDir = 1; }
	else if (pTempUserData->GetPosition().y < tempY - 1) { yDir = -1; }

	const bool isYZero = pClient->sectorIndexY == 0 ? true : false;
	const bool isYMax = pClient->sectorIndexY == 9 ? true : false;

	pClient->possibleSectorCount = 0;

	if (xDir == 0)
	{
		if (yDir == -1)
		{
			if (!isYZero)
			{
				pClient->possibleSectorCount = 1;
				pClient->sectorArr[0] = { pClient->sectorIndexX, pClient->sectorIndexY - 1 };
			}
		}
		else if (yDir == 1)
		{
			if (!isYMax)
			{
				pClient->possibleSectorCount = 1;
				pClient->sectorArr[0] = { pClient->sectorIndexX, pClient->sectorIndexY + 1 };
				
			}
		}
		/* else if (yDir == 0) */
		
		return; //return retVector;
	}
	else if (xDir == 1)
	{
		if (pClient->sectorIndexX == 9)	// X의 끝일 때,
		{
			if (yDir == -1) 
			{ 
				if (!isYZero)
				{
					pClient->possibleSectorCount = 1;
					pClient->sectorArr[0] = { pClient->sectorIndexX, pClient->sectorIndexY - 1 };
					//retVector.emplace_back(std::make_pair(pClient->sectorIndexX, pClient->sectorIndexY - 1));
				}
			}
			else if (yDir == 1)
			{
				if (!isYMax)
				{
					pClient->possibleSectorCount = 1;
					pClient->sectorArr[0] = { pClient->sectorIndexX, pClient->sectorIndexY + 1 };
					//retVector.emplace_back(std::make_pair(pClient->sectorIndexX, pClient->sectorIndexY + 1));

				}
			}
		}
		else
		{
			pClient->possibleSectorCount = 1;
			pClient->sectorArr[0] = { pClient->sectorIndexX + 1, pClient->sectorIndexY};
			//retVector.reserve(4);
			//retVector.emplace_back(std::make_pair(pClient->sectorIndexX + 1, pClient->sectorIndexY));

			if (yDir == -1) 
			{ 
				if (!isYZero)
				{
					pClient->sectorArr[1] = { pClient->sectorIndexX, pClient->sectorIndexY - 1 };
					pClient->sectorArr[2] = { pClient->sectorIndexX + 1, pClient->sectorIndexY - 1};
					pClient->possibleSectorCount = 3;

					//retVector.emplace_back(std::make_pair(pClient->sectorIndexX, pClient->sectorIndexY - 1));
					//retVector.emplace_back(std::make_pair(pClient->sectorIndexX + 1, pClient->sectorIndexY - 1));
				}
			}
			else if (yDir == 1)
			{
				if (!isYMax)
				{
					pClient->sectorArr[1] = { pClient->sectorIndexX, pClient->sectorIndexY + 1 };
					pClient->sectorArr[2] = { pClient->sectorIndexX + 1, pClient->sectorIndexY + 1 };
					pClient->possibleSectorCount = 3;

					//retVector.emplace_back(std::make_pair(pClient->sectorIndexX, pClient->sectorIndexY + 1));
					//retVector.emplace_back(std::make_pair(pClient->sectorIndexX + 1, pClient->sectorIndexY + 1));
				}
			}
		}
		
		return;
	}
	else if (xDir == -1)
	{
		if (pClient->sectorIndexX == 0)
		{
			if (yDir == -1) 
			{ 
				if (!isYZero)
				{
					pClient->possibleSectorCount = 1;
					pClient->sectorArr[0] = { pClient->sectorIndexX, pClient->sectorIndexY - 1 };
					//retVector.emplace_back(std::make_pair(pClient->sectorIndexX, pClient->sectorIndexY - 1));
				}
			}
			else if (yDir == 1)
			{
				if (!isYMax)
				{
					pClient->possibleSectorCount = 1;
					pClient->sectorArr[0] = { pClient->sectorIndexX, pClient->sectorIndexY + 1 };

					//retVector.emplace_back(std::make_pair(pClient->sectorIndexX, pClient->sectorIndexY + 1));
				}
			}
		}
		else
		{
			pClient->possibleSectorCount = 1;
			pClient->sectorArr[0] = { pClient->sectorIndexX - 1, pClient->sectorIndexY };
			//retVector.reserve(4);
			//retVector.emplace_back(std::make_pair(pClient->sectorIndexX - 1, pClient->sectorIndexY));

			if (yDir == -1)
			{
				if (!isYZero)
				{
					pClient->possibleSectorCount = 3;
					pClient->sectorArr[1] = { pClient->sectorIndexX, pClient->sectorIndexY - 1 };
					pClient->sectorArr[2] = { pClient->sectorIndexX - 1, pClient->sectorIndexY - 1 };

					//retVector.emplace_back(std::make_pair(pClient->sectorIndexX, pClient->sectorIndexY - 1));
					//retVector.emplace_back(std::make_pair(pClient->sectorIndexX - 1, pClient->sectorIndexY - 1));
				}
			}
			else if (yDir == 1)
			{
				if (!isYMax)
				{
					pClient->possibleSectorCount = 3;
					pClient->sectorArr[1] = { pClient->sectorIndexX, pClient->sectorIndexY + 1 };
					pClient->sectorArr[2] = { pClient->sectorIndexX - 1, pClient->sectorIndexY + 1 };

					//retVector.emplace_back(std::make_pair(pClient->sectorIndexX, pClient->sectorIndexY + 1));
					//retVector.emplace_back(std::make_pair(pClient->sectorIndexX - 1, pClient->sectorIndexY + 1));
				}
			}
		}
		return ;
	}

	assert(false, "FindPossibleSectors : 여기에 걸릴리가 없는데??");
	return;
}

/*
	RenewViewListInSectors
		- 최신화된 섹터에서, 뷰리스트를 갱신한다.

	!0. 반드시 이 함수가 호출되기 전에, RenewPossibleSectors가 선행되어야 옳은 ViewList를 획득할 수 있습니다.
*/
void Scene::RenewViewListInSectors(SocketInfo* pClient)
{
	sectorCont[pClient->sectorIndexY][pClient->sectorIndexX];
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
	std::cout << "[AfterRecv] 받은 버퍼는" << int(pClient->loadedBuf[1]) << "희망하는 방향은 : " << int(pClient->loadedBuf[1]) << "\n";
#endif
	moveManager->MoveCharacter(pClient);
	moveManager->SendMoveCharacter(pClient, sceneContUnit);
}
