#pragma once

#include "InHeaderDefine.hh"

struct SectorContUnit;
struct ZoneContUnit;

struct SocketInfo;

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

	void JudgeClientWithViewList(SocketInfo*, ZoneContUnit*);

	void SendPutPlayer(SocketInfo* pPutClient, SocketInfo* pRecvClient);
	void SendRemovePlayer(const _ClientKeyType pRemoveClient, SocketInfo* pRecvClient);

	void SendMovePlayer(SocketInfo* pPutClient, SocketInfo* pRecvClient);

private:
	bool IsSeeEachOther(const _PosType, const _PosType, const _PosType, const _PosType) const noexcept;
	bool IsSeeEachOther(const std::pair<_PosType, _PosType>&, const std::pair<_PosType, _PosType>&) const noexcept;

public:
	Sector(const BYTE sectorIndeX, const BYTE sectorIndeY);
	~Sector();

private:
	const BYTE indexX;
	const BYTE indexY;
	const BYTE centerX;
	const BYTE centerY;

	SectorContUnit* sectorContUnit;

public:
	inline constexpr BYTE GetCenterX() const noexcept { return centerX; }
	inline constexpr BYTE GetCenterY() const noexcept { return centerY; }
};