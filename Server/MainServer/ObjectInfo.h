#pragma once

#include "InHeaderDefine.hh"

/*
	몬스터와, NPC가 공통으로 사용할 Object의 정보를 담고 있음.
*/
struct ObjectInfo
{
public:
	ObjectInfo();
	ObjectInfo(_PosType,  _PosType);
	virtual ~ObjectInfo() = default;

public:	// 이럴꺼면 님 왜 이거 class했어요? -> 조용히해 -> 결국 Struct...ㅎ
	_PosType posX;
	_PosType posY;

	_LevelType level;
	
	_JobType job;

	_HpType hp;

	_DamageType damage;

	_SectorIndexType sectorIndexX;
	_SectorIndexType sectorIndexY;

	BYTE possibleSectorCount;
	std::array<std::pair<_SectorIndexType, _SectorIndexType>, 3> sectorArr;
};

/*
	결국 상속을 사용합니다..
*/
struct PlayerObjectInfo : public ObjectInfo
{
	_NicknameType nickname[10];
	_MpType mp;
	_ExpType exp;
	_MoneyType money;
	_RedCountType redCount;
	_BlueCountType blueCount;
	_TreeCountType treeCount;

public:
	PlayerObjectInfo();
	PlayerObjectInfo(_NicknameType*, _PosType, _PosType);
	~PlayerObjectInfo() final = default;
};