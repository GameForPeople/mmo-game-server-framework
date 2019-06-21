#pragma once

#include "InHeaderDefine.hh"

struct SectorContUnit;
struct ZoneContUnit;

struct SocketInfo;
struct ObjectInfo;

/*
	Sector
		- Sector는 시야 처리를 위한 기본 단위입니다.
	
	#0. Sector는 섹터에 포함된 클라이언트의 목록을 갖습니다.
	#1. Zone이나 Manager와 다르게, 해당 클래스는, 내부적으로 처리하지 않고, Client Cont에 대한 부분만 처리합니다!
	#2. #1 사쿠라여, 고냥 Sector에서 처리한다~
*/

class Sector
{
public:
	void Join(SocketInfo*);
	void Exit(SocketInfo*);

	void JoinForNpc(BaseMonster*);
	void ExitForNpc(BaseMonster*);

	void MakeNewViewList(std::unordered_set<_KeyType>&, std::unordered_set<_KeyType>&, SocketInfo*, ZoneContUnit*);
	void MakeNewViewListForNpc(std::unordered_set<_KeyType>&, BaseMonster*, ZoneContUnit*);

	void MakeOldViewListForNpc(std::unordered_set<_KeyType>&, BaseMonster*, ZoneContUnit*);

private:
	//bool IsSeeEachOther(const _PosType, const _PosType, const _PosType, const _PosType) const noexcept;
	//bool IsSeeEachOther(const std::pair<_PosType, _PosType>&, const std::pair<_PosType, _PosType>&) const noexcept;
	bool IsSeeEachOther(const ObjectInfo* const, const ObjectInfo* const) const noexcept;

public:
	Sector(const BYTE sectorIndeX, const BYTE sectorIndeY);
	~Sector();

private:
	const BYTE indexX;
	const BYTE indexY;
	const _PosType centerX;
	const _PosType centerY;

	SectorContUnit* sectorContUnit;

public:
	inline constexpr _PosType GetCenterX() const noexcept { return centerX; }
	inline constexpr _PosType GetCenterY() const noexcept { return centerY; }
};