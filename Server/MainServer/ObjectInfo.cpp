#include "pch.h"

#include "../Define.h"
#include "ServerDefine.h"

#include "ObjectInfo.h"

ObjectInfo::ObjectInfo(_NicknameType inNickname, _PosType inX, _PosType inY)
	: 
	nickname(inNickname),
	posX(inX),
	posY(inY),
	sectorIndexX(GLOBAL_DEFINE::START_SECTOR_X),
	sectorIndexY(GLOBAL_DEFINE::START_SECTOR_Y)
{
}