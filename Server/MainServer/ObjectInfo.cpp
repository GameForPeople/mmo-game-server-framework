#include "pch.h"

#include "../Define.h"
#include "ServerDefine.h"

#include "ObjectInfo.h"

ObjectInfo::ObjectInfo()
	:
	posX(),
	posY(),
	sectorIndexX(),
	sectorIndexY(),
	possibleSectorCount(),
	sectorArr()
{
	// 어짜피 갱신됩니다.
}

ObjectInfo::ObjectInfo(_PosType inX, _PosType inY)
	: 
	posX(inX),
	posY(inY),
	sectorIndexX(),
	sectorIndexY()
{
	// 어짜피 갱신됩니다.
}

//-----------
//
//-----------

PlayerObjectInfo::PlayerObjectInfo() 
	: ObjectInfo()
	, nickname()
{

}

PlayerObjectInfo::PlayerObjectInfo(_NicknameType* inNickname, _PosType inX, _PosType inY)
	: ObjectInfo(inX, inY)
	, nickname()
{
	memcpy(nickname, inNickname, 20);
}