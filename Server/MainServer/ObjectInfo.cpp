#include "pch.h"

#include "../Define.h"
#include "ServerDefine.h"

#include "ObjectInfo.h"

ObjectInfo::ObjectInfo(_KeyType inKey, _PosType inX, _PosType inY)
	: key(inKey),
	posX(inX),
	posY(inY),
	sectorIndexX(GLOBAL_DEFINE::START_SECTOR_X),
	sectorIndexY(GLOBAL_DEFINE::START_SECTOR_Y)
{
}