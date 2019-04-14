#pragma once

struct SectorContUnit;

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
	void InNewClient(SocketInfo*);
	void OutClient(SocketInfo*);

	//void CheckViewList(SocketInfo*);

public:
	Sector(const BYTE X, const BYTE Y);
	~Sector();

private:
	const BYTE indexX;
	const BYTE indexY;
	const BYTE centerX;
	const BYTE centerY;

	SectorContUnit* sectorContUnit;

public:
	_NODISCARD inline constexpr BYTE GetCenterX() noexcept { return centerX; }
	_NODISCARD inline constexpr BYTE GetCenterY() noexcept { return centerY; }
};