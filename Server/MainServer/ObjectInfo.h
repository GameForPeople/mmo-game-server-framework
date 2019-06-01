#pragma once

#include "InHeaderDefine.hh"

/*
	Object의 정보를 담고 있음.

*/
struct ObjectInfo
{
public:
	ObjectInfo(_KeyType,  _PosType,  _PosType);
	~ObjectInfo() = default;

public:	// 이럴꺼면 님 왜 이거 class했어요? -> 조용히해 -> 결국 Struct...ㅎ

	const _KeyType key;	// 이거 필요한거 맞아? 인덱스 매칭할건데...?

	_PosType posX;
	_PosType posY;

	_SectorIndexType sectorIndexX;	// 자신의 섹터로 슥~
	_SectorIndexType sectorIndexY;	// 자신의 섹터로 슥~

	_NicknameType nickname;

	BYTE possibleSectorCount;
	std::array<std::pair<_SectorIndexType, _SectorIndexType>, 3> sectorArr;
};
