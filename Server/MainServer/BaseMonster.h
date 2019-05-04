#pragma once

#include "InHeaderDefine.hh"

class BaseMonster 
{
public:
	BaseMonster( _MonsterKeyType,  _PosType,  _PosType);
	~BaseMonster() = default;

public:	// 이럴꺼면 님 왜 이거 class했어요? -> 조용히해

	const _MonsterKeyType key;	// 이거 필요한거 맞아? 인덱스 매칭할건데...?

	_PosType posX;
	_PosType posY;

	BYTE sectorIndexX;	// 자신의 섹터로 슥~
	BYTE sectorIndexY;	// 자신의 섹터로 슥~

	BYTE possibleSectorCount;	// 검사해야하는 섹터 개수, 최대 3을 초과할 수 없음.
	std::array<std::pair<BYTE, BYTE>, 3> sectorArr;
};