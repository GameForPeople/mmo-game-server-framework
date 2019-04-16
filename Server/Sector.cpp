#include "pch.h"
#include "Define.h"
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

void Sector::InNewClient(SocketInfo* pOutNewClient)
{
	sectorContUnit->wrlock.lock(); //+++++++++++++++++++++++++++++++++++++1
	sectorContUnit->clientCont.emplace(pOutNewClient->clientKey);
	sectorContUnit->wrlock.unlock(); //-----------------------------------0

	pOutNewClient->sectorIndexX = indexX;
	pOutNewClient->sectorIndexY = indexY;
}

void Sector::OutClient(SocketInfo* pInClient)
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
	CheckViewList
	 - Sector 내에서, 인자로 전달된 클라이언트와의 viewList의 여부 검사.

	 !0. 미친 이상한 구조 떔시, ZoneContUnit을 갖고 옵니다... 잘못쓰면 성능 간다간다 훅간다!
	 !1. 서로의 viewList에 넣을 때, 동기화 백프로 꺠질걸?? ConCurrency가 아닌 Lock을 써야 될꺼 같어.
*/
void Sector::CheckViewList(SocketInfo* pClient, ZoneContUnit* pZoneContUnit)
{
	// 이부분 나중에 동접 오지면 오버헤드 왕 오집니다. 다른 방안을 생각해야합니다.
	pZoneContUnit->wrlock.lock_shared();	//++++++++++++++++++++++++++++1 Zone : Read Lock!
	auto oldZoneCont = pZoneContUnit->clientCont;
	pZoneContUnit->wrlock.unlock_shared();	//----------------------------0 Zone : Read unLock!

	sectorContUnit->wrlock.lock_shared();	//++++++++++++++++++++++++++++1	Sector : Read Lock!
	for (auto& otherKey : sectorContUnit->clientCont)
	{
		if (otherKey == pClient->clientKey) continue;

		if (IsSeeEachOther(pClient->userData->GetPosition(), oldZoneCont[otherKey].second->userData->GetPosition()))
		{
			// 서로 보입니다.
			if (pClient->viewList.find(otherKey) == pClient->viewList.end())
			{
				// 서로 보이고, 서로 모르는 사이였을 때.
				SendPutPlayer(pClient, oldZoneCont[otherKey].second);
				SendPutPlayer(oldZoneCont[otherKey].second, pClient);

				// 서로의 뷰 리스트에 추가할 때... 문제가 될 수 있습니다.
				pClient->viewList.insert(otherKey);
				oldZoneCont[otherKey].second->viewList.insert(pClient->clientKey);
			}
			else
			{
				//서로 보이고, 서로 아는 사이였을 때,

				// 나는 내가 알아서 할게 너 나 바뀐거 받아라 얌마!
				SendMovePlayer(pClient, oldZoneCont[otherKey].second);
			}
		}
		else
		{
			// 서로 안보입니다.
			if (pClient->viewList.find(otherKey) != pClient->viewList.end())
			{
				// 서로 안 보이고, 서로 원래 알던 클라이언트였을 때.
				SendRemovePlayer(pClient->clientKey, oldZoneCont[otherKey].second);
				SendRemovePlayer(otherKey, pClient);

				// 서로의 뷰 리스트에서 삭제할 때... 문제가 될 수 있습니다.
				// 언세이프 보이지?? 살벌하다살벌해
				pClient->viewList.unsafe_erase(otherKey);
				oldZoneCont[otherKey].second->viewList.unsafe_erase(pClient->clientKey);
			}
			//else
			//{
			//	// 서로 안 보이고, 서로 원래 모르던 클라이언트였을 때.
			//}
		}
	}
	sectorContUnit->wrlock.unlock_shared();	//----------------------------0	Sector : Read Lock!
}

bool Sector::IsSeeEachOther(const Position2D& inAPosition, const Position2D& inBPosition) noexcept
{
	if (GLOBAL_DEFINE::VIEW_DISTANCE < abs(inAPosition.x - inBPosition.x)) return false;
	if (GLOBAL_DEFINE::VIEW_DISTANCE < abs(inAPosition.y - inBPosition.y)) return false;
	return true;
}

void Sector::SendPutPlayer(SocketInfo* pPutClient, SocketInfo* pRecvClient)
{
	PACKET_DATA::SC::PutPlayer packet(
		pPutClient->clientKey,
		pPutClient->userData->GetPosition().x,
		pPutClient->userData->GetPosition().y
	);

	NETWORK_UTIL::SendPacket(pRecvClient, reinterpret_cast<char*>(&packet));
}

void Sector::SendRemovePlayer(const _ClientKeyType pRemoveClientID, SocketInfo* pRecvClient)
{
	PACKET_DATA::SC::RemovePlayer packet(
		pRemoveClientID
	);

	NETWORK_UTIL::SendPacket(pRecvClient, reinterpret_cast<char*>(&packet));
}

void Sector::SendMovePlayer(SocketInfo* pMovedClientKey, SocketInfo* pRecvClient) 
{
	PACKET_DATA::SC::Position packet(
		pMovedClientKey->clientKey,
		pMovedClientKey->userData->GetPosition().x,
		pMovedClientKey->userData->GetPosition().y
	);

	NETWORK_UTIL::SendPacket(pRecvClient, reinterpret_cast<char*>(&packet));
}