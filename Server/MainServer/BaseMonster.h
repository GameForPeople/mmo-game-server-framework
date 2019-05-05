#pragma once

#include "InHeaderDefine.hh"

struct ObjectInfo;

class BaseMonster {
public:
	BaseMonster(_KeyType, _PosType, _PosType);
	~BaseMonster();
	ObjectInfo* objectInfo;
};
