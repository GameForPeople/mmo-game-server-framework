#include "pch.h"

#include "ObjectInfo.h"
#include "BaseMonster.h"

BaseMonster::BaseMonster(_KeyType inKey, _PosType inX, _PosType inY)
{
	objectInfo = new ObjectInfo(inKey, inX, inY);
}

BaseMonster::~BaseMonster()
{
	delete objectInfo;
}