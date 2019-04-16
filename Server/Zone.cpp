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

#include "Zone.h"

Zone::Zone() : 
	moveManager(nullptr),
	connectManager(nullptr),
	zoneContUnit(nullptr),
	recvFunctionArr()
{
	InitManagers();
	InitClientCont();
	InitSector();
	InitFunctions();
}

Zone::~Zone()
{
	delete[] recvFunctionArr;
	delete[] zoneContUnit;
}

/*
	Zone::InitManagers()
		- GamsServer의 생성자에서 호출되며, 각 매니저들의 초기화를 담당합니다.
*/
void Zone::InitManagers()
{
	moveManager = std::make_unique<MoveManager>();
	connectManager = std::make_unique<ConnectManager>();
}

/*
	Zone::InitClientCont()
		- GamsServer의 생성자에서 호출되며, 클라이언트 컨테이너들을 초기화합니다.
*/
void Zone::InitClientCont()
{
	zoneContUnit = new ZoneContUnit;
	zoneContUnit->clientCont.reserve(GLOBAL_DEFINE::MAX_CLIENT);

	for (int i = 0; i < GLOBAL_DEFINE::MAX_CLIENT; ++i)
	{
		zoneContUnit->clientCont.emplace_back(false, nullptr);
	}
}

/*
	Zone::InitFunctions()
		- GamsServer의 생성자에서 호출되며, 게임과 관련된 데이터들의 초기화를 담당합니다.
*/
void Zone::InitFunctions()
{
	recvFunctionArr = new std::function<void(Zone&, SocketInfo*)>[PACKET_TYPE::CS::ENUM_SIZE];
	recvFunctionArr[PACKET_TYPE::CS::MOVE] = &Zone::RecvCharacterMove;
}

/*
	Zone::InitSectorCont()
		- GamsServer의 생성자에서 호출되며, 섹터들을 초기화합니다.
*/
void Zone::InitSector()
{
	// 상수 무결성 검사
	if(GLOBAL_DEFINE::MAX_HEIGHT != GLOBAL_DEFINE::MAX_WIDTH) std::cout << "### Zone의 상수가 비정상적입니다. 확인해주세요.";
	if((int)((GLOBAL_DEFINE::MAX_HEIGHT - 1) / GLOBAL_DEFINE::SECTOR_DISTANCE) 
		== (int)((GLOBAL_DEFINE::MAX_HEIGHT + 1) / GLOBAL_DEFINE::SECTOR_DISTANCE)) std::cout << "### Zone의 상수가 비정상적입니다. 확인해주세요.";

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
	Zone::ProcessRecvData()
		- 받은 데이터들을 함수와 연결해줍니다.
*/
void Zone::ProcessPacket(SocketInfo* pClient)
{
	recvFunctionArr[(pClient->loadedBuf[1]) % (PACKET_TYPE::CS::ENUM_SIZE)](*this, pClient);
}

/*
	Zone::InNewClient()
		- connectManager에서 해당 함수 처리하도록 변경.
*/
/*std::optional<SocketInfo*>*/ 
_ClientNode /* == std::pair<bool, SocketInfo*>*/ Zone::InNewClient()
{
	if (auto retNode = connectManager->InNewClient(zoneContUnit, this)
		; retNode.first)
	{
		//최초 Sector에 클라이언트 삽입.
		sectorCont[5][5].InNewClient(retNode.second);

		// InitViewAndSector으로 래핑됨.

		// 둘러볼 지역 결정하고 -> 소켓을 포트에 등록 후, 나중에 사귈껍니다.
		//RenewPossibleSectors(retNode.second);

		// 친구들 새로 사귀고 -> 소켓을 포트에 등록 후, 나중에 사귈껍니다.
		//RenewViewListInSectors(retNode.second);

		return retNode;
	}
	else return retNode;
}

void Zone::InitViewAndSector(SocketInfo* pClient)
{
	RenewPossibleSectors(pClient);
	RenewViewListInSectors(pClient);
}

/*
	Zone::OutClient()
		- connectManager에서 해당 함수 처리하도록 변경.
*/
void Zone::OutClient(SocketInfo* pOutClient)
{
	// 섹터 컨테이너에서 내 정보를 지워주고
	sectorCont[pOutClient->sectorIndexY][pOutClient->sectorIndexX].OutClient(pOutClient);
	
	// 내 ViewList의 Client에게 나 나간다고 알려주고.
	connectManager->OutClient(pOutClient, zoneContUnit);
}

/*
	FindPossibleSectors? CheckPossibleSectors?
		- 현재 캐릭터의 섹터와 위치를 검사하여, 시야 체크를 해야하는 섹터를 검사합니다.

	?0. 기존의 지역변수를 생성하여, 리턴하는 방식에서, SocketInfo의 멤버변수를 두는 방식으로 변경하는게 날꺼 같은디?
*/
void Zone::RenewPossibleSectors(SocketInfo* pClient)
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
void Zone::RenewViewListInSectors(SocketInfo* pClient)
{
	sectorCont[pClient->sectorIndexY][pClient->sectorIndexX].CheckViewList(pClient, zoneContUnit);

	for (int i = 0; i < pClient->possibleSectorCount; ++i)
	{
		sectorCont[pClient->sectorArr[i].second][pClient->sectorArr[i].first].CheckViewList(pClient, zoneContUnit);
	}
}

void Zone::RenewClientSector(SocketInfo* pClient)
{
	bool isNeedToChangeSector{ false };

	if (static_cast<BYTE>(pClient->userData->GetPosition().x / 10) != pClient->sectorIndexX) isNeedToChangeSector = true;
	if (static_cast<BYTE>(pClient->userData->GetPosition().y / 10) != pClient->sectorIndexY) isNeedToChangeSector = true;

	if (isNeedToChangeSector == false) 	return;
	else
	{
		sectorCont[pClient->sectorIndexY][pClient->sectorIndexX].OutClient(pClient);
		sectorCont[pClient->sectorIndexY][pClient->sectorIndexX].InNewClient(pClient);
	}
}

/*
	Zone::RecvCharacterMove(SocketInfo* pClient)
		- 클라이언트로부터 CharacterMove를 받았을 경우, 호출되는 함수.

	#0. 해당 클라이언트의 Move를 먼저 처리합니다.
	#1. Zone에 존재하는 자신을 포함한 모든 클라이언트에게 ID값과 위치값을 전송합니다.
*/
void Zone::RecvCharacterMove(SocketInfo* pClient)
{
#ifdef _DEV_MODE_
	std::cout << "[AfterRecv] 받은 버퍼는" << int(pClient->loadedBuf[1]) << "희망하는 방향은 : " << int(pClient->loadedBuf[2]) << "\n";
#endif
	moveManager->MoveCharacter(pClient);

	// 스스로에게 전송.
	PACKET_DATA::SC::Position packet(
		pClient->clientKey,
		pClient->userData->GetPosition().x,
		pClient->userData->GetPosition().y
	);
	NETWORK_UTIL::SendPacket(pClient, reinterpret_cast<char*>(&packet));

	RenewClientSector(pClient);
	RenewPossibleSectors(pClient);
	RenewViewListInSectors(pClient);

	//moveManager->SendMoveCharacter(pClient, zoneContUnit);
	//sectorCont[pClient->sectorIndexY][pClient->sectorIndexX].RecvCharacterMove(pClient);
}
