#pragma once

#include "InHeaderDefine.hh"

struct SocketInfo;
struct ZoneContUnit;

class Zone;

class ConnectManager
{
public:
	ConnectManager() = default;
	~ConnectManager() = default;

	ConnectManager(const ConnectManager&) = delete;
	ConnectManager& operator=(const ConnectManager&) = delete;

public:
	_ClientNode LogInToZone(ZoneContUnit* inClientContUnit, Zone*);
	void LogOutToZone(SocketInfo*, ZoneContUnit* inClientContUnit);

private:
	void SendRemovePlayerInOuttedClientViewList(SocketInfo* pOutClient, ZoneContUnit* inClientCont);
	
	// Sector로 이사갔습니다.
	//void SendPutPlayer(SocketInfo* pPutClient, ZoneContUnit* inClientCont);
private:
	//std::shared_mutex connectLock;
};