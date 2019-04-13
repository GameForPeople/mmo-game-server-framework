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

SectorContUnit* Sector::GetSectorContUnit()
{
	return sectorContUnit;
}

void Sector::InNewClient(SocketInfo* pOutNewClient)
{
	sectorContUnit->wrlock.lock();
	sectorContUnit->clientCont.emplace_back(pOutNewClient->clientKey);
	sectorContUnit->wrlock.unlock();

	pOutNewClient->sectorIndexX = indexX;
	pOutNewClient->sectorIndexY = indexY;
}

void Sector::OutClient(SocketInfo* pInClient)
{
	sectorContUnit->wrlock.lock();

	sectorContUnit->wrlock.unlock();
}