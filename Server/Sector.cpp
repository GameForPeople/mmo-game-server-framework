#include "pch.h"
#include "ServerDefine.h"

#include "ClientContUnit.h"
#include "MemoryUnit.h"

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
	sectorContUnit->clientCont.emplace_back(pOutNewClient->clientKey);
	sectorContUnit->wrlock.unlock(); //-----------------------------------0

	pOutNewClient->sectorIndexX = indexX;
	pOutNewClient->sectorIndexY = indexY;
}

void Sector::OutClient(SocketInfo* pInClient)
{
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