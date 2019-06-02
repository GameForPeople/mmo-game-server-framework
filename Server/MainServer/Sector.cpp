#include "pch.h"
#include "../Define.h"
#include "ServerDefine.h"

#include "ClientContUnit.h"
#include "MemoryUnit.h"
#include "UserData.h"

#include "ObjectInfo.h"
#include "BaseMonster.h"

#include "Sector.h"

Sector::Sector(const BYTE inX, const BYTE inY)
	: indexX(inX)
	, indexY(inY)
	, centerX(inX * GLOBAL_DEFINE::SECTOR_DISTANCE + GLOBAL_DEFINE::SECTOR_HALF_DISTANCE)
	, centerY(inY * GLOBAL_DEFINE::SECTOR_DISTANCE + GLOBAL_DEFINE::SECTOR_HALF_DISTANCE)
	, sectorContUnit(new SectorContUnit)
{
}

Sector::~Sector()
{
	delete sectorContUnit;
}

/*
	Join
		- 섹터에 새로운 클라이언트가 들어옵니다.
*/
void Sector::Join(SocketInfo* pClient)
{
	sectorContUnit->wrlock.lock(); //+++++++++++++++++++++++++++++++++++++1
	sectorContUnit->clientCont.emplace(pClient->key);
	sectorContUnit->wrlock.unlock(); //-----------------------------------0

	pClient->objectInfo->sectorIndexX = indexX;
	pClient->objectInfo->sectorIndexY = indexY;
}

/*
	Exit
		- 섹터에 있던 클라이언트가 나갑니다.
*/
void Sector::Exit(SocketInfo* pInClient)
{
	// 지역 정보에서 나를 지워주고.
	sectorContUnit->wrlock.lock(); //+++++++++++++++++++++++++++++++++++++1

	// 이게 무슨 미친놈의 코드야;
	//for (auto iter = sectorContUnit->clientCont.begin()
	//	; iter != sectorContUnit->clientCont.end()
	//	; ++iter)
	//
	//	if (pInClient->key == *iter)
	//	{
	//		sectorContUnit->clientCont.erase(iter);
	//		break;
	//	}
	//
	sectorContUnit->clientCont.erase(pInClient->key);

	sectorContUnit->wrlock.unlock(); //-----------------------------------0
}

void Sector::JoinForNpc(BaseMonster* pClientObject)
{
	sectorContUnit->monsterlock.lock(); //+++++++++++++++++++++++++++++++++++++1
	sectorContUnit->monsterCont.emplace(pClientObject->key);
	sectorContUnit->monsterlock.unlock(); //-----------------------------------0

	pClientObject->objectInfo->sectorIndexX = indexX;
	pClientObject->objectInfo->sectorIndexY = indexY;
}

void Sector::ExitForNpc(BaseMonster* pMonster)
{
	sectorContUnit->monsterlock.lock(); //+++++++++++++++++++++++++++++++++++++1
	
	// 니도 이게 무슨 코드야;
	//for (auto iter = sectorContUnit->monsterCont.begin()
	//	; iter != sectorContUnit->monsterCont.end()
	//	; ++iter)
	//{
	//	if (pInClient->key == *iter)
	//	{
	//		sectorContUnit->monsterCont.erase(iter);
	//		break;
	//	}
	//}
	sectorContUnit->monsterCont.erase(pMonster->key);

	sectorContUnit->monsterlock.unlock(); //-----------------------------------0
}

/*
	JudgeClientInViewList
	 - Sector의 컨테이너에서 볼수 있는 클라이언트를 판단하고, viewList의 여부에 따라 처리합니다..

	 !0. 미친 이상한 구조 떔시, ZoneContUnit을 갖고 옵니다... 잘못쓰면 성능 간다간다 훅간다!
	 !1. 서로의 viewList에 넣을 때, 동기화 백프로 꺠질걸?? ConCurrency가 아닌 Lock을 써야 될꺼 같어.
	 !2. 내부에서 네트워크 함수가 호출됩니다.
*/
void Sector::JudgeClientWithViewList(SocketInfo* pClient, ZoneContUnit* pZoneContUnit)
{
	if (sectorContUnit->clientCont.size() > 1)
	{
		auto oldViewList = pClient->viewList;
		std::unordered_set<_ClientKeyType> newViewList;

		sectorContUnit->wrlock.lock_shared();	//++++++++++++++++++++++++++++1	Sector : Read Lock!
		for (auto/*&*/ otherKey : sectorContUnit->clientCont)
		{
			if (otherKey == pClient->key) continue;

			SocketInfo* pOtherClient = pZoneContUnit->clientContArr[otherKey];

			//if (!isOn) continue; // 이럴일 없을듯해.

			if (IsSeeEachOther(pClient->objectInfo, pOtherClient->objectInfo))
			{
				// 서로 보입니다.
				newViewList.insert(otherKey);
				/*
				if (pClient->viewList.find(otherKey) == pClient->viewList.end())
				{
					// 서로 보이고, 서로 모르는 사이였을 때.
					SendPutPlayer(pClient->objectInfo, pOtherClient);
					SendPutPlayer(pOtherClient->objectInfo, pClient);

					// 서로의 뷰 리스트에 추가할 때... 문제가 될 수 있습니다.
					pClient->viewList.insert(otherKey);
					pOtherClient->viewList.insert(pClient->objectInfo->key);
				}
				else
				{
					//서로 보이고, 서로 아는 사이였을 때,

					// 나는 내가 알아서 할게 너 나 바뀐거 받아라 얌마!
					SendMovePlayer(pClient->objectInfo, pOtherClient);
				}
				*/
			}
			/*
			else
			{
				// 서로 안보입니다.
				if (pClient->viewList.find(otherKey) != pClient->viewList.end())
				{
					// 서로 안 보이고, 서로 원래 알던 클라이언트였을 때.
					SendRemovePlayer(pClient->objectInfo->key, pOtherClient);
					SendRemovePlayer(otherKey, pClient);

					// 서로의 뷰 리스트에서 삭제할 때... 문제가 될 수 있습니다.
					// 언세이프 보이지?? 살벌하다살벌해
					pClient->viewList.unsafe_erase(otherKey);
					pOtherClient->viewList.unsafe_erase(pClient->objectInfo->key);
				}
				//else
				//{
				//	// 서로 안 보이고, 서로 원래 모르던 클라이언트였을 때.
				//}
			}
			*/

		}
		sectorContUnit->wrlock.unlock_shared();	//----------------------------0	Sector : Read Lock!

		for (auto otherClientKey : newViewList)
		{
			if (oldViewList.count(otherClientKey) != 0)
				// 새로 시야에 들어옴.
			{
				SocketInfo* pOtherClient = pZoneContUnit->clientContArr[otherClientKey];

				// 추가 처리가 요청됩니다. 동기화가 안될 가능성이 큽니다.
				pClient->viewList.insert(otherClientKey);
				SendPutPlayer(pOtherClient->objectInfo, pClient);
				//

				if (pOtherClient->viewList.count(pClient->key) != 0)
				{
					SendMovePlayer(pClient, pOtherClient);
				}
				else
				{
					pOtherClient->viewList.insert(pClient->key);
					SendPutPlayer(pClient, pOtherClient);
				}
			}
		}
	}

	if (sectorContUnit->monsterCont.size() > 0)
	{
		auto oldMonsterViewList = pClient->monsterViewList;
		std::unordered_set<_ClientKeyType> newMonsterViewList;

		sectorContUnit->monsterlock.lock_shared(); //++++++++++++++++++++++++++++1	Sector : Read Lock!
		for (auto/*&*/ otherKey : sectorContUnit->monsterCont)
		{
			auto pMonster = pZoneContUnit->monsterCont[otherKey - BIT_CONVERTER::NOT_PLAYER_INT];

			if (IsSeeEachOther(pClient->objectInfo, pMonster->objectInfo))
			{
				// 서로 보입니다.
				newMonsterViewList.insert(otherKey);
				/*
				if (pClient->monsterViewList.find(otherKey) == pClient->monsterViewList.end())
				{
					// 서로 보이고, 서로 모르는 사이였을 때.
					SendPutPlayer(pMonster->objectInfo, pClient);

					// 서로의 뷰 리스트에 추가할 때... 문제가 될 수 있습니다.
					pClient->monsterViewList.insert(otherKey);
				}
				else
				{
					//서로 보이고, 서로 아는 사이였을 때,

					// 나는 내가 알아서 할게 너 나 바뀐거 받아라 얌마!
					SendMovePlayer(pMonster->objectInfo, pClient);
				}
				*/
			}
			/*
			else
			{
				// 서로 안보입니다.
				if (pClient->monsterViewList.find(otherKey) != pClient->monsterViewList.end())
				{
					// 서로 안 보이고, 서로 원래 알던 클라이언트였을 때.
					//SendRemovePlayer(pClient->objectInfo->key, pOtherClient);
					SendRemovePlayer(otherKey, pClient);

					// 서로의 뷰 리스트에서 삭제할 때... 문제가 될 수 있습니다.
					// 언세이프 보이지?? 살벌하다살벌해
					pClient->monsterViewList.unsafe_erase(otherKey);
				}
				//else
				//{
				//	// 서로 안 보이고, 서로 원래 모르던 클라이언트였을 때.
				//}
			}
			*/
		}
		sectorContUnit->monsterlock.unlock_shared(); //----------------------------0	Sector : Read Lock!
	
		for (auto otherMonsterKey : newMonsterViewList)
		{
			if (oldMonsterViewList.count(otherMonsterKey) != 0)
				// 새로 시야에 들어옴.
			{
				auto pMonster = pZoneContUnit->monsterCont[otherMonsterKey - BIT_CONVERTER::NOT_PLAYER_INT];

				pClient->monsterViewList.insert(otherMonsterKey);
				SendPutPlayer(pMonster, pClient);
				
				// WAKE UP 처리가 필요합니다.

			}
		}
	}
}

bool Sector::JudgeClientWithViewListForNpc(BaseMonster* pMonster, ZoneContUnit* pZoneContUnit)
{
	// 대충 0명이느냐 아니느냐 검사
	if (sectorContUnit->clientCont.size() == 0) {
		/*std::cout << "텅비어있습니다!" << std::endl; */ return false; //이게 많아야할텐데? }
	}

	bool retValue = false;
	std::vector<_KeyType> localViewList;

	sectorContUnit->wrlock.lock_shared();	//++++++++++++++++++++++++++++1	Sector : Read Lock!
	for (auto/*&*/ otherKey : sectorContUnit->clientCont)
	{
		//auto[isOn, pOtherClient] = pZoneContUnit->FindClient(otherKey /*- BIT_CONVERTER::NOT_PLAYER_INT*/);
		//if (!isOn) continue; // ? 왜 없댱

		if (IsSeeEachOther(pMonster->objectInfo, pZoneContUnit->clientContArr[otherKey]->objectInfo))
		{
			localViewList.emplace_back(otherKey);
			// 서로 보입니다.
			//if (pOtherClient->monsterViewList.find(pClient->key /* - BIT_CONVERTER::NOT_PLAYER_INT*/) == pOtherClient->monsterViewList.end())
			//{
			//	// 서로 보이고, 서로 모르는 사이였을 때.
			//	SendPutPlayer(pClient, pOtherClient);
			//	//SendPutPlayer(pOtherClient->objectInfo, pClient);
			//
			//	// 서로의 뷰 리스트에 추가할 때... 문제가 될 수 있습니다.
			//	//pClient->viewList.insert(otherKey);
			//	pOtherClient->monsterViewList.insert(pClient->key);
			//}
			//else
			//{
			//	//서로 보이고, 서로 아는 사이였을 때,
			//
			//	// 나는 이미 움직였어, 너 나 바뀐거 받아라 얌마!
			//	SendMovePlayer(pClient, pOtherClient);
			//}
		}
		//else
		//{
		//	// 서로 안보입니다.
		//	if (pOtherClient->monsterViewList.find(pClient->key) != pOtherClient->monsterViewList.end())
		//	{
		//		// 서로 안 보이고, 서로 원래 알던 클라이언트였을 때.
		//
		//		SendRemovePlayer(pClient->key, pOtherClient);
		//		//SendRemovePlayer(otherKey, pClient);
		//
		//		// 서로의 뷰 리스트에서 삭제할 때... 문제가 될 수 있습니다.
		//		// 언세이프 보이지?? 살벌하다살벌해
		//		pOtherClient->monsterViewList.unsafe_erase(pClient->key);
		//	}
		//	//else
		//	//{
		//	//	// 서로 안 보이고, 서로 원래 모르던 클라이언트였을 때.
		//	//}
		//}
	}
	sectorContUnit->wrlock.unlock_shared();	//----------------------------0	Sector : Read Lock!

	for (auto clientKey : localViewList)
	{
		if (pZoneContUnit->clientContArr[clientKey]->monsterViewList.count(pMonster->key))
		{
			SendMovePlayer( pMonster->objectInfo, pZoneContUnit->clientContArr[clientKey]);
		}
		else
		{
			pZoneContUnit->clientContArr[clientKey]->monsterViewList.insert(pMonster->key);
			SendPutPlayer(pMonster, pZoneContUnit->clientContArr[clientKey]);
		}
	}

	return retValue;
}

/*
	IsSeeEachOther
		- 마! 서로 볼수 있나 없나! 위치값 둘다 내나 봐라 마!
*/
//bool Sector::IsSeeEachOther(const _PosType aPosX, const _PosType aPosY, const _PosType bPosX, const _PosType bPosY) const noexcept
//{
//	if (GLOBAL_DEFINE::VIEW_DISTANCE < abs(aPosX - bPosX)) return false;
//	if (GLOBAL_DEFINE::VIEW_DISTANCE < abs(aPosY - bPosY)) return false;
//	return true;
//}
//
//bool Sector::IsSeeEachOther(const std::pair<_PosType, _PosType>& inAPosition, const std::pair<_PosType, _PosType>& inBPosition) const noexcept
//{
//	if (GLOBAL_DEFINE::VIEW_DISTANCE < abs(inAPosition.first - inBPosition.first)) return false;
//	if (GLOBAL_DEFINE::VIEW_DISTANCE < abs(inAPosition.second - inBPosition.second)) return false;
//	return true;
//}

bool Sector::IsSeeEachOther(const ObjectInfo* const objectA, const ObjectInfo* const objectB) const noexcept
{
	if (GLOBAL_DEFINE::VIEW_DISTANCE < abs(objectA->posX - objectB->posX)) return false;
	if (GLOBAL_DEFINE::VIEW_DISTANCE < abs(objectA->posY - objectB->posY)) return false;
	return true;
}

template <class OBJECT>
void Sector::SendPutPlayer(OBJECT* pPutObject, SocketInfo* pRecvClient)
{
	PACKET_DATA::MAIN_TO_CLIENT::PutPlayer packet(
		pPutObject->key,
		//pPutClient->userData->GetPosition().x,
		//pPutClient->userData->GetPosition().y
		pPutObject->objectInfo->posX,
		pPutObject->objectInfo->posY
	);

	NETWORK_UTIL::SendPacket(pRecvClient, reinterpret_cast<char*>(&packet));
}

void Sector::SendRemovePlayer(const _ClientKeyType pRemoveClientID, SocketInfo* pRecvClient)
{
	PACKET_DATA::MAIN_TO_CLIENT::RemovePlayer packet(pRemoveClientID);

	NETWORK_UTIL::SendPacket(pRecvClient, reinterpret_cast<char*>(&packet));
}

template <class OBJECT>
void Sector::SendMovePlayer(OBJECT* pMovedObject, SocketInfo* pRecvClient)
{
	PACKET_DATA::MAIN_TO_CLIENT::Position packet(
		pMovedClient->key,
		//pMovedClientKey->userData->GetPosition().x,
		//pMovedClientKey->userData->GetPosition().y
		pMovedClient->objectInfo->posX,
		pMovedClient->objectInfo->posY
	);

	NETWORK_UTIL::SendPacket(pRecvClient, reinterpret_cast<char*>(&packet));
}