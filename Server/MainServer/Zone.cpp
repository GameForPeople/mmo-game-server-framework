#include "pch.h"
#include "../Define.h"
#include "ServerDefine.h"

#include "TimerManager.h"
#include "ConnectManager.h"
#include "MoveManager.h"
#include "ChatManager.h"

#include "ClientContUnit.h"

#include "UserData.h"

#include "BaseMonster.h"

#include "MemoryUnit.h"
#include "SendMemoryPool.h"

#include "Sector.h"

#include "ObjectInfo.h"

#include "BaseMonster.h"
#include "MonsterLoader.h" 
#include "MonsterModelManager.h" 

#include "Zone.h"

Zone::Zone() : 
	//connectManager(nullptr),
	moveManager(nullptr),
	monsterModelManager(nullptr),
	sectorCont(),
	//recvFunctionArr(nullptr),
	zoneContUnit(nullptr)
{
	// 이 순서가 변경될 경우, 서버가 동작하지 않습니다.
	InitManagers();
	InitSector();
	InitZoneCont();
}

Zone::~Zone()
{
	//delete[] recvFunctionArr;
	delete[] zoneContUnit;
}

/*
	Zone::InitManagers()
		- GamsServer의 생성자에서 호출되며, 각 매니저들의 초기화를 담당합니다.
*/
void Zone::InitManagers()
{
	moveManager = std::make_unique<MoveManager>();
	//connectManager = std::make_unique<ConnectManager>();
	monsterModelManager = std::make_unique<MonsterModelManager>();
}

/*
	Zone::InitSectorCont()
		- GamsServer의 생성자에서 호출되며, 섹터들을 초기화합니다.
*/
void Zone::InitSector()
{
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
	Zone::InitZoneCont()
		- GamsServer의 생성자에서 호출되며, 클라이언트 컨테이너들을 초기화합니다.

	!0. InitSector가 호출된후 호출되어야 합니다.
*/
void Zone::InitZoneCont()
{
	zoneContUnit = new ZoneContUnit;

	_KeyType tempIndex = BIT_CONVERTER::NOT_PLAYER_INT;

	std::cout << "#. 몬스터 "<< zoneContUnit->monsterCont.size() << " 를 할당중입니다...";

	auto tempMapDate = moveManager->GetMapData();

	// 추후 쓰레드 분할
	for (auto& monster : zoneContUnit->monsterCont)
	{
		_PosType tempPosX;
		_PosType tempPosY;
		
		do 
		{
			tempPosX = rand() % GLOBAL_DEFINE::MAX_WIDTH;
			tempPosY = rand() % GLOBAL_DEFINE::MAX_HEIGHT;
		} 
		while (tempMapDate[tempPosY][tempPosX] == false);

		monster = new BaseMonster(tempIndex++, tempPosX, tempPosY, monsterModelManager->GetMonsterModel(MONSTER_TYPE::SLIME));

		sectorCont[tempPosY / GLOBAL_DEFINE::SECTOR_DISTANCE][tempPosX / GLOBAL_DEFINE::SECTOR_DISTANCE].JoinForNpc(monster);
		RenewPossibleSectors(monster->objectInfo);
	}

	std::cout << "    할당이 종료되었습니다." << std::endl;
}


void Zone::ProcessTimerUnit(const int timerManagerContIndex)
{
	concurrency::concurrent_queue<TimerUnit*>* tempCont = TimerManager::GetInstance()->GetTimerContWithIndex(timerManagerContIndex);

	TimerUnit* pUnit = nullptr;
	while (tempCont->try_pop(pUnit))
	{
		switch (auto[objectType, index] = BIT_CONVERTER::WhatIsYourTypeAndRealKey(pUnit->objectKey); objectType)
		{
		case BIT_CONVERTER::OBJECT_TYPE::PLAYER:
			switch (pUnit->timerType)
			{
				case (TIMER_TYPE::PUSH_OLD_KEY):
				{
					ConnectManager::GetInstance()->PushOldKey(pUnit->objectKey);
					TimerManager::GetInstance()->PushTimerUnit(pUnit);
					break;
				}
				case (TIMER_TYPE::CC_NODAMAGE):
				{
					//이거는 항상 보장됨. if - CAS 문 안걸어도됨.
					zoneContUnit->clientContArr[index]->objectInfo->noDamageFlag = false;
					TimerManager::GetInstance()->PushTimerUnit(pUnit);
					break;
				}
				case (TIMER_TYPE::PLAYER_MOVE):
				{

					break;
				}
				case (TIMER_TYPE::PLAYER_ATTACK):
				{

					break;
				}
				case (TIMER_TYPE::SELF_HEAL):
				{
					auto playerObjectInfo = zoneContUnit->clientContArr[index]->objectInfo;
					if (zoneContUnit->clientContArr[index]->objectInfo->hp > 0)
					{
						const int maxHp = JOB::GetMaxHP(playerObjectInfo->job, playerObjectInfo->level);

						while (7) 
						{
							unsigned short oldHp = zoneContUnit->clientContArr[index]->objectInfo->hp;
							unsigned short newHp = zoneContUnit->clientContArr[index]->objectInfo->hp + (maxHp / 10);

							if (oldHp == 0)
							{
								TimerManager::GetInstance()->PushTimerUnit(pUnit);
								break;
							}
							else if (oldHp == maxHp) 
							{
								TimerManager::GetInstance()->PushTimerUnit(pUnit);
								break;
							}
							else if (newHp > maxHp) newHp = maxHp;

							if (ATOMIC_UTIL::T_CAS(&(zoneContUnit->clientContArr[index]->objectInfo->hp), oldHp, newHp))
							{
								// 피회복한거 보내야함.
								TimerManager::GetInstance()->AddTimerEvent(pUnit, TIME::SELF_HEAL);
								break;
							}
						}
					}
					else
					{
						TimerManager::GetInstance()->PushTimerUnit(pUnit);
					}
					break;
				}
				case (TIMER_TYPE::ITEM_HP):
				{
					zoneContUnit->clientContArr[index]->objectInfo->redTickCount.fetch_sub(1);

					auto playerObjectInfo = zoneContUnit->clientContArr[index]->objectInfo;
					if (zoneContUnit->clientContArr[index]->objectInfo->hp > 0)
					{
						const int maxHp = JOB::GetMaxHP(playerObjectInfo->job, playerObjectInfo->level);

						while (7)
						{
							unsigned short oldHp = zoneContUnit->clientContArr[index]->objectInfo->hp;
							unsigned short newHp = zoneContUnit->clientContArr[index]->objectInfo->hp + ITEM::RED_PER_TICK;

							if (oldHp == 0)
							{
								TimerManager::GetInstance()->PushTimerUnit(pUnit);
								zoneContUnit->clientContArr[index]->objectInfo->redTickCount = 0;
								break;
							}
							else if (oldHp == maxHp)
							{
								TimerManager::GetInstance()->PushTimerUnit(pUnit);
								zoneContUnit->clientContArr[index]->objectInfo->redTickCount = 0;
								break;
							}
							else if (newHp > maxHp) newHp = maxHp;

							if (ATOMIC_UTIL::T_CAS(&(zoneContUnit->clientContArr[index]->objectInfo->hp), oldHp, newHp))
							{
								// 피회복한거 보내야함.
								TimerManager::GetInstance()->AddTimerEvent(pUnit, TIME::ITEM_HP);
								break;
							}
						}
					}
					else
					{
						TimerManager::GetInstance()->PushTimerUnit(pUnit);
						zoneContUnit->clientContArr[index]->objectInfo->redTickCount = 0;
					}
					break;
				}
				case (TIMER_TYPE::ITEM_MP):
				{
					zoneContUnit->clientContArr[index]->objectInfo->redTickCount.fetch_sub(1);

					auto playerObjectInfo = zoneContUnit->clientContArr[index]->objectInfo;

					const int maxMp = JOB::GetMaxMP(playerObjectInfo->job, playerObjectInfo->level);

					while (7)
					{
						unsigned short oldMp = zoneContUnit->clientContArr[index]->objectInfo->mp;
						unsigned short newMp = zoneContUnit->clientContArr[index]->objectInfo->mp + ITEM::BLUE_PER_TICK;

						if (oldMp == maxMp)
						{
							TimerManager::GetInstance()->PushTimerUnit(pUnit);
							zoneContUnit->clientContArr[index]->objectInfo->blueTickCount = 0;
							break;
						}
						else if (newMp > maxMp) newMp = maxMp;

						if (ATOMIC_UTIL::T_CAS(&(zoneContUnit->clientContArr[index]->objectInfo->hp), oldMp, newMp))
						{
							// 마나회복한거 보내야함.
							TimerManager::GetInstance()->AddTimerEvent(pUnit, TIME::ITEM_MP);
							break;
						}
					}

					break;
				}
			}
			break;
		case BIT_CONVERTER::OBJECT_TYPE::MONSTER:
			switch (pUnit->timerType)
			{
				case (TIMER_TYPE::NPC_MOVE):
				{
					BaseMonster* tempBaseMonster = zoneContUnit->monsterCont[index];

					//현재 PossibleSector의 경우에는, 이전 움직임의 PossibleSector이므로 유효함
					std::unordered_set<_KeyType> oldViewList;
					MakeOldViewListForNpc(oldViewList, zoneContUnit->monsterCont[index]);

					moveManager->MoveRandom(tempBaseMonster->objectInfo);	// 랜덤으로 움직이고
					RenewSelfSectorForNpc(tempBaseMonster);					// 혹시 움직여서 섹터가 바뀐듯하면 바뀐 섹터로 적용해주고
					RenewPossibleSectors(tempBaseMonster->objectInfo);		// 현재 섹터의 위치에서, 탐색해야하는 섹터들을 정해주고

					// 주변에 클라이언트가 없습니다. 이동을 종료합니다.
					if (RenewViewListInSectorsForNpc(oldViewList, zoneContUnit->monsterCont[index]))
					{
						TimerManager::GetInstance()->AddTimerEvent(pUnit, TIME::SLIME_MOVE);
					}
					else
					{
						ATOMIC_UTIL::T_CAS(&(tempBaseMonster->isSleep), true, false); // 결과는 딱히 안중요해!
						TimerManager::GetInstance()->PushTimerUnit(pUnit);
					}
					break;
				}
				case (TIMER_TYPE::NPC_ATTACK):
				{
					break;
				}
				case (TIMER_TYPE::SKILL_1_COOLTIME):
				{
					break;
				}
				case (TIMER_TYPE::SKILL_2_COOLTIME):
				{
					break;
				}
				case (TIMER_TYPE::REVIVAL):
				{
					break;
				}
				case (TIMER_TYPE::CC_FAINT):
				{
					zoneContUnit->monsterCont[index]->objectInfo->faintTick.fetch_sub(1);
					TimerManager::GetInstance()->PushTimerUnit(pUnit);
					break;
				}
				case (TIMER_TYPE::CC_FREEZE):
				{
					zoneContUnit->monsterCont[index]->freezeTick.fetch_sub(1);
					TimerManager::GetInstance()->PushTimerUnit(pUnit);
					break;
				}
				case (TIMER_TYPE::CC_ELECTRIC):
				{
					zoneContUnit->monsterCont[index]->electricTick.fetch_sub(1);
					TimerManager::GetInstance()->PushTimerUnit(pUnit);
					break;
				}
				case (TIMER_TYPE::CC_BURN_3):
				{
					while (7) 
					{
						unsigned short oldHp = zoneContUnit->monsterCont[index]->objectInfo->hp;
						unsigned short newHp = oldHp - (oldHp / 100);

						if (oldHp > 0)
						{
							if (ATOMIC_UTIL::T_CAS(&(zoneContUnit->monsterCont[index]->objectInfo->hp), oldHp, newHp))
							{
								pUnit->timerType = TIMER_TYPE::CC_BURN_2;
								TimerManager::GetInstance()->AddTimerEvent(pUnit, TIME::SECOND);
								break;
							}
						}
						else
						{
							TimerManager::GetInstance()->PushTimerUnit(pUnit);
							break;
						}
					}
					break;
				}
				case (TIMER_TYPE::CC_BURN_2):
				{
					while (7)
					{
						unsigned short oldHp = zoneContUnit->monsterCont[index]->objectInfo->hp;
						unsigned short newHp = oldHp - (oldHp / 100);

						if (oldHp > 0)
						{
							if (ATOMIC_UTIL::T_CAS(&(zoneContUnit->monsterCont[index]->objectInfo->hp), oldHp, newHp))
							{
								pUnit->timerType = TIMER_TYPE::CC_BURN_1;
								TimerManager::GetInstance()->AddTimerEvent(pUnit, TIME::SECOND);
								break;
							}
						}
						else
						{
							TimerManager::GetInstance()->PushTimerUnit(pUnit);
							break;
						}
					}
					break;
				}
				case (TIMER_TYPE::CC_BURN_1):
				{
					while (7)
					{
						unsigned short oldHp = zoneContUnit->monsterCont[index]->objectInfo->hp;
						unsigned short newHp = oldHp - (oldHp / 100);

						if (oldHp > 0)
						{
							if (ATOMIC_UTIL::T_CAS(&(zoneContUnit->monsterCont[index]->objectInfo->hp), oldHp, newHp))
							{
								TimerManager::GetInstance()->PushTimerUnit(pUnit);
								break;
							}
						}
						else
						{
							TimerManager::GetInstance()->PushTimerUnit(pUnit);
							break;
						}
					}
					break;
				}
				default:
					assert(false, L"[ERROR] 정의되지 않은 Tmier Unit이 실행되었습니다. 서버를 종료합니다.");

					// 디버그가 아닐 경우! 실제는 그냥 반납.
					TimerManager::GetInstance()->PushTimerUnit(pUnit);

					break;
			}
			break;
		default:
			break;
		}
	}
}

/*
	Zone::TryToEnter()
		- 해당 존으로의 입장을 시도합니다.

	#0. Zone의 connectManager에서 입장 여부 판단
	#1. 입장 성공 시, 최초 위치(섹터 5, 5)에 클라이언트 삽입.
	#2.	입장 실패 시, 아무런 짓도 하지 않음
*/
/*std::optional<SocketInfo*>*/ 
//std::pair<bool, SocketInfo*> /* == std::pair<bool, SocketInfo*>*/ Zone::TryToEnter()
//{
//	if (auto retNode = connectManager->LogInToZone(zoneContUnit, this)
//		; retNode.first)
//	{
//		//최초 Sector에 클라이언트 삽입.
//		sectorCont[GLOBAL_DEFINE::START_SECTOR_Y][GLOBAL_DEFINE::START_SECTOR_X].Join(retNode.second->objectInfo);
//
//		// InitViewAndSector에서 래핑되며, Accept Process에서 해당 클라이언트 소켓을 IOCP 등록 후, 호출함
//		//{
//			// 둘러볼 지역 결정하고 -> 소켓을 포트에 등록 후, 나중에 사귈껍니다.
//			//RenewPossibleSectors(retNode.second);
//
//		// 친구들 새로 사귀고 -> 소켓을 포트에 등록 후, 나중에 사귈껍니다.
//			//RenewViewListInSectors(retNode.second);
//		//}
//
//		return retNode;
//	}
//	else return retNode;
//}

/*
	Zone::Enter()
		- 해당 존 및 섹터에 들어가고, 보이는 애들애게 알립니다.

	#0.connectManager에서 해당 함수 처리하도록 변경.은 절로가 없어짐~
*/
void Zone::Enter(SocketInfo* pEnteredClient)
{
	// 섹터 컨테이너에서 내 정보를 먼저 넣어주고.
	sectorCont[pEnteredClient->objectInfo->posY / GLOBAL_DEFINE::SECTOR_DISTANCE][pEnteredClient->objectInfo->posX / GLOBAL_DEFINE::SECTOR_DISTANCE].Join(pEnteredClient);

	// PossibleSector와 View 처리
	InitViewAndSector(pEnteredClient);
}

/*
	Zone::Exit()
		- 해당 존에서 나갑니다.

	#0.connectManager에서 해당 함수 처리하도록 변경.
*/
void Zone::Exit(SocketInfo* pOutClient)
{
	// 현재 포함되어 있는, 섹터 컨테이너에서 내 정보를 지워주고
	sectorCont[pOutClient->objectInfo->sectorIndexY][pOutClient->objectInfo->sectorIndexX].Exit(pOutClient);
	
	// 내 ViewList의 Client에게 나 나간다고 알려주고.
	//connectManager->LogOutToZone(pOutClient, zoneContUnit);

	for (auto iter : pOutClient->viewList)
	{
		zoneContUnit->clientContArr[iter]->viewList.erase(pOutClient->key);
		NETWORK_UTIL::SEND::SendRemovePlayer(pOutClient->key, zoneContUnit->clientContArr[iter]);
	}
}

/*
	Zone::InitViewAndSector()
		- Zone의 Enter 후, 최초로 Sector와 ViewList를 갱신합니다.

	!0. 해당 두 함수들은 네트워크 함수를 포함하고 있습니다.
		- IOCP에 소켓을 등록한 후에, 호출되어야 합니다.
*/
void Zone::InitViewAndSector(SocketInfo* pClient)
{
	RenewPossibleSectors(pClient->objectInfo);
	RenewViewListInSectors(pClient);
}


/*
	FindPossibleSectors? CheckPossibleSectors?
		- 현재 캐릭터의 섹터와 위치를 검사하여, 시야 체크를 해야하는 섹터를 검사합니다.

	?0. 기존의 지역변수를 생성하여, 리턴하는 방식에서, SocketInfo의 멤버변수를 두는 방식으로 변경하는게 날꺼 같은디? -> 그렇게 했다가 이게 뭐야 ㅡ
*/
void Zone::RenewPossibleSectors(ObjectInfo* pClient)
{
	// 로컬 변수를 리턴하는 코드에서, 멤버 변수를 활용하여 구현하는 방식으로 변경.

	const _PosType nowSectorCenterX = sectorCont[pClient->sectorIndexY][pClient->sectorIndexX].GetCenterX();
	const _PosType nowSectorCenterY = sectorCont[pClient->sectorIndexY][pClient->sectorIndexX].GetCenterY();

	char xDir = 0;	// x 섹터의 판단 방향
	char yDir = 0;	// y 섹터의 판단 방향

	/*
		Sector's Size = 10 example
		View Length = 3

		possible Other Sector Count

		0   1   2 | 3   4   5   6 | 7   8   9
		1	      |				  |
		 -1,1 = 3 |		0,1 = 1	  |	 1,1 = 3
		2		  |				  |
		--------------------------------------
		3		  |				  |
				  |				  |
		4		  |				  |
		 -1,0 = 1 |		0,0 = 0	  |	 1,0 = 1
		5		  |				  |
				  |				  |
		6		  |				  |
		--------------------------------------
		7		  |				  |
				  |				  |
		8		  |				  |
		 -1,-1 = 3|		0,-1 = 1  |	 1,-1 = 3
		9		  |				  |
	*/

	if (pClient->posX > nowSectorCenterX + GLOBAL_DEFINE::SECTOR_PLUS_LIMIT_DISTANCE) { xDir = 1; }
	else if (pClient->posX < nowSectorCenterX - GLOBAL_DEFINE::SECTOR_MINUS_LIMIT_DISTANCE) { xDir = -1; }

	if (pClient->posY > nowSectorCenterY + GLOBAL_DEFINE::SECTOR_PLUS_LIMIT_DISTANCE) { yDir = 1; }
	else if (pClient->posY < nowSectorCenterY - GLOBAL_DEFINE::SECTOR_MINUS_LIMIT_DISTANCE) { yDir = -1; }

	const bool isYZero = pClient->sectorIndexY == 0 ? true : false;
	const bool isYMax = pClient->sectorIndexY == (GLOBAL_DEFINE::SECTOR_END_POSITION) ? true : false;

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
		if (pClient->sectorIndexX == GLOBAL_DEFINE::SECTOR_END_POSITION)	// X의 끝일 때,
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

	assert(false, L"FindPossibleSectors : 여기에 걸릴리가 없는데??");
	return;
}

/*
	RenewViewListInSectors
		- RenewPossibleSectors에서 최신화한 섹터들에서, 뷰리스트를 갱신한다.

	!0. 반드시 이 함수가 호출되기 전에, RenewPossibleSectors가 선행되어야 옳은 ViewList를 획득할 수 있습니다.
*/
void Zone::RenewViewListInSectors(SocketInfo* pClient)
{
	pClient->viewListLock.lock_shared();
	auto oldViewList = pClient->viewList;
	pClient->viewListLock.unlock_shared();

	std::unordered_set<_ClientKeyType> newViewList;

	pClient->monsterViewListLock.lock_shared();
	auto oldMonsterViewList = pClient->monsterViewList;
	pClient->monsterViewListLock.unlock_shared();

	std::unordered_set<_ClientKeyType> newMonsterViewList;
	
	// Make NewViewList
	sectorCont[pClient->objectInfo->sectorIndexY][pClient->objectInfo->sectorIndexX].MakeNewViewList(newViewList, newMonsterViewList, pClient, zoneContUnit);

	for (int i = 0; i < pClient->objectInfo->possibleSectorCount; ++i)
	{
		sectorCont[pClient->objectInfo->sectorArr[i].second][pClient->objectInfo->sectorArr[i].first].MakeNewViewList(newViewList, newMonsterViewList, pClient, zoneContUnit);
	}

	// Client
	for (auto otherClientKey : newViewList)
	{
		if (SocketInfo* pOtherClient = zoneContUnit->clientContArr[otherClientKey];
			oldViewList.count(otherClientKey) != 0)
		{
			pOtherClient->viewListLock.lock();
			if (0 != pOtherClient->viewList.count(pClient->key))
			{
				pOtherClient->viewListLock.unlock();
				NETWORK_UTIL::SEND::SendMovePlayer<SocketInfo, PACKET_DATA::MAIN_TO_CLIENT::Position>(pClient, pOtherClient);
			}
			else
			{
				pOtherClient->viewList.insert(pClient->key);
				pOtherClient->viewListLock.unlock();
				NETWORK_UTIL::SEND::SendPutPlayer<SocketInfo, PACKET_DATA::MAIN_TO_CLIENT::PutPlayer>(pClient, pOtherClient);
			}
		}
		else
			// 새로 시야에 들어옴.
		{
			pClient->viewListLock.lock();
			pClient->viewList.insert(otherClientKey);
			pClient->viewListLock.unlock();

			NETWORK_UTIL::SEND::SendPutPlayer<SocketInfo, PACKET_DATA::MAIN_TO_CLIENT::PutPlayer>(pClient, pOtherClient);

			pOtherClient->viewListLock.lock();
			if (0 != pOtherClient->viewList.count(pClient->key)) {
				pOtherClient->viewListLock.unlock();
				NETWORK_UTIL::SEND::SendMovePlayer<SocketInfo, PACKET_DATA::MAIN_TO_CLIENT::Position>(pOtherClient, pClient);
			}
			else {
				pOtherClient->viewList.insert(pClient->key);
				pOtherClient->viewListLock.unlock();
				NETWORK_UTIL::SEND::SendPutPlayer<SocketInfo, PACKET_DATA::MAIN_TO_CLIENT::PutPlayer>(pOtherClient, pClient);
			}
		}
	}

	for (auto otherClientKey : oldViewList)
	{
		if (newViewList.count(otherClientKey) != 0) continue;

		pClient->viewListLock.lock();
		pClient->viewList.erase(otherClientKey);
		pClient->viewListLock.unlock();

		NETWORK_UTIL::SEND::SendRemovePlayer(otherClientKey, pClient);

		SocketInfo* pOtherClient = zoneContUnit->clientContArr[otherClientKey];

		pOtherClient->viewListLock.lock();
		if (0 != pOtherClient->viewList.count(pClient->key))
		{
			pOtherClient->viewList.erase(pClient->key);
			 pOtherClient->viewListLock.unlock();
			NETWORK_UTIL::SEND::SendRemovePlayer(pClient->key, pOtherClient);
		}
		else pOtherClient->viewListLock.unlock();
	}

	// Monster

	for (auto otherMonsterKey : newMonsterViewList)
	{
		if (oldMonsterViewList.count(otherMonsterKey) != 0)
		{
			// 기존에 있던 친구들.
			// 몬스터 친구들 안녀엉! 나 움직인다아~~~!!
		}
		else
		{
			// 새로 시야에 들어옴.
			auto pMonster = zoneContUnit->monsterCont[otherMonsterKey - BIT_CONVERTER::NOT_PLAYER_INT];
			pClient->monsterViewListLock.lock();
			pClient->monsterViewList.insert(otherMonsterKey);
			pClient->monsterViewListLock.unlock();

			NETWORK_UTIL::SEND::SendPutPlayer<BaseMonster, PACKET_DATA::MAIN_TO_CLIENT::PutPlayer>(pMonster, pClient);

			if (ATOMIC_UTIL::T_CAS(&(pMonster->isSleep), false, true))
			{
				// 이동 타이머를 등록해줌.
				auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
				timerUnit->timerType = TIMER_TYPE::NPC_MOVE;
				timerUnit->objectKey = pMonster->key;
				TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::SLIME_MOVE);
			}
			//else  // 이미 true일 경우 할 것 없어!
		}
	}

	for (auto otherMonsterKey : oldMonsterViewList)
	{
		auto pMonster = zoneContUnit->monsterCont[otherMonsterKey - BIT_CONVERTER::NOT_PLAYER_INT];

		if (0 != newMonsterViewList.count(otherMonsterKey)) continue;

		pClient->monsterViewListLock.lock();
		pClient->monsterViewList.erase(otherMonsterKey);
		pClient->monsterViewListLock.unlock();

		NETWORK_UTIL::SEND::SendRemovePlayer(otherMonsterKey, pClient);
	}
}

/*
	RenewViewListInSectorsForNPC
		- RenewPossibleSectors에서 최신화한 섹터들에서, 뷰리스트를 갱신한다.

	!0. 반드시 이 함수가 호출되기 전에, RenewPossibleSectors가 선행되어야 옳은 ViewList를 획득할 수 있습니다.
*/
bool Zone::RenewViewListInSectorsForNpc(const std::unordered_set<_KeyType>& oldViewList, BaseMonster* pMonster)
{
	bool retValue = false;
	auto pObjectInfo = pMonster->objectInfo;
	std::unordered_set<_KeyType> newViewList;

	// make newViewList

	sectorCont[pObjectInfo->sectorIndexY][pObjectInfo->sectorIndexX].MakeNewViewListForNpc(newViewList, pMonster, zoneContUnit);

	for (int i = 0; i < pObjectInfo->possibleSectorCount; ++i)
	{
		sectorCont[pObjectInfo->sectorArr[i].second][pObjectInfo->sectorArr[i].first].MakeNewViewListForNpc(newViewList, pMonster, zoneContUnit);
	}

	// 

	_KeyType monsterKey = pMonster->key;
	for (auto clientKey : oldViewList)
	{
		if (auto pOtherClient = zoneContUnit->clientContArr[clientKey]; 
			0 != newViewList.count(clientKey)) {
			pOtherClient->monsterViewListLock.lock();
			
			if (pOtherClient->monsterViewList.count(monsterKey)) {
				pOtherClient->monsterViewListLock.unlock();
				NETWORK_UTIL::SEND::SendMovePlayer<BaseMonster, PACKET_DATA::MAIN_TO_CLIENT::Position>(pMonster, pOtherClient);
			}
			else {
				pOtherClient->monsterViewList.insert(monsterKey);
				pOtherClient->monsterViewListLock.unlock();
				NETWORK_UTIL::SEND::SendPutPlayer< BaseMonster, PACKET_DATA::MAIN_TO_CLIENT::PutPlayer>(pMonster, pOtherClient);
			}
		}
		else {
			pOtherClient->monsterViewListLock.lock();

			if (0 < pOtherClient->monsterViewList.count(monsterKey)) {
				pOtherClient->monsterViewList.erase(monsterKey);
				pOtherClient->monsterViewListLock.unlock();
				NETWORK_UTIL::SEND::SendRemovePlayer(monsterKey, pOtherClient);
			}
			else pOtherClient->monsterViewListLock.unlock();
		}
	}
	for (auto clientKey : newViewList)
	{
		if (oldViewList.count(clientKey) == 0)
		{
			auto pOtherClient = zoneContUnit->clientContArr[clientKey];

			pOtherClient->monsterViewListLock.lock();

			if (0 == pOtherClient->monsterViewList.count(monsterKey)) {
				pOtherClient->monsterViewList.insert(monsterKey);
				pOtherClient->monsterViewListLock.unlock();
				NETWORK_UTIL::SEND::SendPutPlayer<BaseMonster, PACKET_DATA::MAIN_TO_CLIENT::PutPlayer>(pMonster, zoneContUnit->clientContArr[clientKey]);
			}
			else {
				pOtherClient->monsterViewListLock.unlock();
				NETWORK_UTIL::SEND::SendMovePlayer<BaseMonster, PACKET_DATA::MAIN_TO_CLIENT::Position>(pMonster, zoneContUnit->clientContArr[clientKey]);
			}
		}
	}
	// 야 너 주변에 이제 아무도 없다! 쉬어 고생했어 ㅠ
	return static_cast<bool>(newViewList.size());
}

void Zone::MakeOldViewListForNpc(std::unordered_set<_KeyType>& retVewList, BaseMonster* pMonster)
{
	sectorCont[pMonster->objectInfo->sectorIndexY][pMonster->objectInfo->sectorIndexX].MakeOldViewListForNpc(retVewList, pMonster, zoneContUnit);

	for (int i = 0; i < pMonster->objectInfo->possibleSectorCount; ++i)
	{
		sectorCont[pMonster->objectInfo->sectorArr[i].second][pMonster->objectInfo->sectorArr[i].first].MakeOldViewListForNpc(retVewList, pMonster, zoneContUnit);
	}
}


/*
	RenewClientSector
		- 클라이언트가 이동한 후, 다른 섹터로의 이동 여부를 판단합니다.

	!0. 반드시 이 함수가 호출되기 전에, RenewPossibleSectors가 선행되어야 옳은 ViewList를 획득할 수 있습니다.
*/
void Zone::RenewSelfSector(SocketInfo* pClient)
{
	ObjectInfo* tempObjectInfo = pClient->objectInfo;

	_SectorIndexType tempX = static_cast<_SectorIndexType>(tempObjectInfo->posX / GLOBAL_DEFINE::SECTOR_DISTANCE);
	_SectorIndexType tempY = static_cast<_SectorIndexType>(tempObjectInfo->posY / GLOBAL_DEFINE::SECTOR_DISTANCE);

	if (tempX != tempObjectInfo->sectorIndexX || tempY != tempObjectInfo->sectorIndexY)
	{
		sectorCont[tempObjectInfo->sectorIndexY][tempObjectInfo->sectorIndexX].Exit(pClient);
		//sectorCont[pClient->sectorIndexY][pClient->sectorIndexX].Join(pClient);
		sectorCont[tempY][tempX].Join(pClient);
	}
	else return;
}

void Zone::RenewSelfSectorForNpc(BaseMonster* pMonster)
{
	ObjectInfo* tempObjectInfo = pMonster->objectInfo;

	_SectorIndexType tempX = static_cast<_SectorIndexType>(tempObjectInfo->posX / GLOBAL_DEFINE::SECTOR_DISTANCE);
	_SectorIndexType tempY = static_cast<_SectorIndexType>(tempObjectInfo->posY / GLOBAL_DEFINE::SECTOR_DISTANCE);

	if (tempX != tempObjectInfo->sectorIndexX || tempY != tempObjectInfo->sectorIndexY)
	{
		sectorCont[tempObjectInfo->sectorIndexY][tempObjectInfo->sectorIndexX].ExitForNpc(pMonster);
		sectorCont[tempY][tempX].JoinForNpc(pMonster);
		//sectorCont[pClient->sectorIndexY][pClient->sectorIndexX].JoinForNpc(pClient);
	}
	else return;
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
	//std::cout << "[AfterRecv] 받은 버퍼는" << int(pClient->loadedBuf[1]) << "희망하는 방향은 : " << int(pClient->loadedBuf[2]) << "\n";
#endif

	if (moveManager->MoveCharacter(pClient))
	{
		// 스스로에게 전송.
		NETWORK_UTIL::SEND::SendMovePlayer<SocketInfo, PACKET_DATA::MAIN_TO_CLIENT::Position>(pClient, pClient);
		//PACKET_DATA::MAIN_TO_CLIENT::Position packet(
		//	pClient->key,
		//	pClient->objectInfo->posX,
		//	pClient->objectInfo->posY
		//);
		//NETWORK_UTIL::SendPacket(pClient, reinterpret_cast<char*>(&packet));

		RenewSelfSector(pClient);
		RenewPossibleSectors(pClient->objectInfo);
		RenewViewListInSectors(pClient);
	}
}

void Zone::RecvChat(SocketInfo* pClient)
{
#ifdef _DEV_MODE_
	std::cout << "[AfterRecv] 채팅 버퍼를 받았습니다. \n";
#endif
	//chatManager->ChatProcess(pClient, zoneContUnit);
}


const std::vector<std::vector<bool>>& Zone::GetMapData()
{
	return moveManager->GetMapData();
}