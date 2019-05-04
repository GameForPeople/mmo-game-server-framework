#include "pch.h"
#include "../Define.h"
#include "ServerDefine.h"

#include "ClientContUnit.h"
#include "MemoryUnit.h"
#include "UserData.h"


#include "Sector.h"

Sector::Sector(const BYTE inX, const BYTE inY)
	: indexX(inX)
	, indexY(inY)
	, centerX(inX * GLOBAL_DEFINE::SECTOR_DISTANCE + GLOBAL_DEFINE::SECTOR_HALF_DISTANCE)
	, centerY(inY* GLOBAL_DEFINE::SECTOR_DISTANCE + GLOBAL_DEFINE::SECTOR_HALF_DISTANCE)
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
void Sector::Join(SocketInfo* pOutNewClient)
{
	sectorContUnit->wrlock.lock(); //+++++++++++++++++++++++++++++++++++++1
	sectorContUnit->clientCont.emplace(pOutNewClient->clientKey);
	sectorContUnit->wrlock.unlock(); //-----------------------------------0

	pOutNewClient->sectorIndexX = indexX;
	pOutNewClient->sectorIndexY = indexY;
}

/*
	Exit
		- 섹터에 있던 클라이언트가 나갑니다.
*/
void Sector::Exit(SocketInfo* pInClient)
{
	// 지역 정보에서 나를 지워주고.
	sectorContUnit->wrlock.lock(); //+++++++++++++++++++++++++++++++++++++1

	for (auto iter = sectorContUnit->clientCont.begin()
		; iter != sectorContUnit->clientCont.end()
		; ++iter)
	{
		if (pInClient->clientKey == *iter)
		{
			sectorContUnit->clientCont.erase(iter);
			break;
		}
	}

	sectorContUnit->wrlock.unlock(); //-----------------------------------0
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
	sectorContUnit->wrlock.lock_shared();	//++++++++++++++++++++++++++++1	Sector : Read Lock!
	for (auto& otherKey : sectorContUnit->clientCont)
	{
		if (otherKey == pClient->clientKey) continue;

		auto[isOn, pOtherClient] = pZoneContUnit->FindClient(otherKey);

		if (!isOn) continue;

		if (IsSeeEachOther(pClient->posX, pClient->posY, pOtherClient->posX, pOtherClient->posY))
		{
			// 서로 보입니다.
			if (pClient->viewList.find(otherKey) == pClient->viewList.end())
			{
				// 서로 보이고, 서로 모르는 사이였을 때.
				SendPutPlayer(pClient, pOtherClient);
				SendPutPlayer(pOtherClient, pClient);

				// 서로의 뷰 리스트에 추가할 때... 문제가 될 수 있습니다.
				pClient->viewList.insert(otherKey);
				pOtherClient->viewList.insert(pClient->clientKey);
			}
			else
			{
				//서로 보이고, 서로 아는 사이였을 때,

				// 나는 내가 알아서 할게 너 나 바뀐거 받아라 얌마!
				SendMovePlayer(pClient, pOtherClient);
			}
		}
		else
		{
			// 서로 안보입니다.
			if (pClient->viewList.find(otherKey) != pClient->viewList.end())
			{
				// 서로 안 보이고, 서로 원래 알던 클라이언트였을 때.
				SendRemovePlayer(pClient->clientKey, pOtherClient);
				SendRemovePlayer(otherKey, pClient);

				// 서로의 뷰 리스트에서 삭제할 때... 문제가 될 수 있습니다.
				// 언세이프 보이지?? 살벌하다살벌해
				pClient->viewList.unsafe_erase(otherKey);
				pOtherClient->viewList.unsafe_erase(pClient->clientKey);
			}
			//else
			//{
			//	// 서로 안 보이고, 서로 원래 모르던 클라이언트였을 때.
			//}
		}
	}
	sectorContUnit->wrlock.unlock_shared();	//----------------------------0	Sector : Read Lock!
}

/*
	IsSeeEachOther
		- 마! 서로 볼수 있나 없나! 위치값 둘다 내나 봐라 마!
*/
bool Sector::IsSeeEachOther(const _PosType aPosX, const _PosType aPosY, const _PosType bPosX, const _PosType bPosY) const noexcept
{
	if (GLOBAL_DEFINE::VIEW_DISTANCE < abs(aPosX - bPosX)) return false;
	if (GLOBAL_DEFINE::VIEW_DISTANCE < abs(aPosY - bPosY)) return false;
	return true;
}

bool Sector::IsSeeEachOther(const std::pair<_PosType, _PosType>& inAPosition, const std::pair<_PosType, _PosType>& inBPosition) const noexcept
{
	if (GLOBAL_DEFINE::VIEW_DISTANCE < abs(inAPosition.first - inBPosition.first)) return false;
	if (GLOBAL_DEFINE::VIEW_DISTANCE < abs(inAPosition.second - inBPosition.second)) return false;
	return true;
}

void Sector::SendPutPlayer(SocketInfo* pPutClient, SocketInfo* pRecvClient)
{
	PACKET_DATA::MAIN_TO_CLIENT::PutPlayer packet(
		pPutClient->clientKey,
		//pPutClient->userData->GetPosition().x,
		//pPutClient->userData->GetPosition().y
		pPutClient->posX,
		pPutClient->posY
	);

	NETWORK_UTIL::SendPacket(pRecvClient, reinterpret_cast<char*>(&packet));
}

void Sector::SendRemovePlayer(const _ClientKeyType pRemoveClientID, SocketInfo* pRecvClient)
{
	PACKET_DATA::MAIN_TO_CLIENT::RemovePlayer packet(pRemoveClientID);

	NETWORK_UTIL::SendPacket(pRecvClient, reinterpret_cast<char*>(&packet));
}

void Sector::SendMovePlayer(SocketInfo* pMovedClientKey, SocketInfo* pRecvClient) 
{
	PACKET_DATA::MAIN_TO_CLIENT::Position packet(
		pMovedClientKey->clientKey,
		//pMovedClientKey->userData->GetPosition().x,
		//pMovedClientKey->userData->GetPosition().y
		pMovedClientKey->posX,
		pMovedClientKey->posY
	);

	NETWORK_UTIL::SendPacket(pRecvClient, reinterpret_cast<char*>(&packet));
}