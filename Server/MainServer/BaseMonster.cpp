#include "pch.h"

#include "../Define.h"
#include "ServerDefine.h"

#include "BaseMonster.h"

BaseMonster::BaseMonster(_MonsterKeyType inKey, _PosType inX, _PosType inY)
	: key(inKey),
	posX(inX),
	posY(inY)
{
}