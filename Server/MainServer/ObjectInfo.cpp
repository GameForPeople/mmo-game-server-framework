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
	sectorArr(),
	faintTick(0),
	burnTick(0),
	damage()
{
	// 어짜피 갱신됩니다.
}

ObjectInfo::ObjectInfo(_PosType inX, _PosType inY)
	: 
	posX(inX),
	posY(inY),
	sectorIndexX(),
	sectorIndexY(),
	faintTick(0),
	//burnTick(0),
	damage()
{
	// 어짜피 갱신됩니다.
}

//-----------
//
//-----------

PlayerObjectInfo::PlayerObjectInfo() 
	: ObjectInfo()
	, nickname()
	//, noDamageTick()
	//, selfHealTick()
{

}

PlayerObjectInfo::PlayerObjectInfo(_NicknameType* inNickname, _PosType inX, _PosType inY)
	: ObjectInfo(inX, inY)
	, nickname()
	, noDamageFlag(false)	// 한번
	, selfHealFlag(false)	// 피 
	, moveFlag(true)
	, attackFlag(true)
	, skill1Flag(false)
	, skill2Flag(false)
	, redTickCount(0)
	, blueTickCount(0)
{
	memcpy(nickname, inNickname, 20);
}