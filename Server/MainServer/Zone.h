#pragma once

#include "InHeaderDefine.hh"

struct SocketInfo;
struct MemoryUnit;
struct TimerUnit;

class ConnectManager;
class MoveManager;

class Sector;

struct ZoneContUnit;

/*
	Zone
		- GameServer(GameWorld)를 구성하는 단위 객체입니다.
	
	#0. 씐에 접속한 Client의 객체들의 컨테이너를 갖고, 관리합니다.
	#1. 네트워크 함수들은 GameServer의 함수가 아닌, 여기서 호출합니다.
	#2. 공간 분할의 단위, 기준은 멤버 변수인 Sector입니다.
*/

class Zone
{
public:
	void ProcessPacket(SocketInfo* pClient);
	void ProcessTimerUnit(TimerUnit* pUnit);

	Zone();
	~Zone();

public: // ConnectManager
	std::pair<bool, SocketInfo*> /*std::optional<SocketInfo*>*/ TryToEnter();
	void Exit(SocketInfo*);
	void InitViewAndSector(SocketInfo* );

private:
	void InitManagers();
	void InitClientCont();
	void InitFunctions();
	void InitSector();

private:
	void RenewClientSector(SocketInfo* pClient);
	void RenewPossibleSectors(SocketInfo* pClient);
	void RenewViewListInSectors(SocketInfo* pClient);

	// MoveManager
	void RecvCharacterMove(SocketInfo* pClient);
	void RecvChat(SocketInfo* pClient);

private:
	std::unique_ptr<ConnectManager> connectManager;
	std::unique_ptr<MoveManager> moveManager;

	std::vector<std::vector<Sector>> sectorCont;

	ZoneContUnit* zoneContUnit;
	std::function<void(Zone&, SocketInfo*)>* recvFunctionArr;
};
