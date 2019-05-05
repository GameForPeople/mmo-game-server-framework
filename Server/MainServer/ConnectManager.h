#pragma once

#include "InHeaderDefine.hh"

struct SocketInfo;
struct ZoneContUnit;

class Zone;

class ConnectManager
{
public:
	ConnectManager();
	~ConnectManager() = default;

	ConnectManager(const ConnectManager&) = delete;
	ConnectManager& operator=(const ConnectManager&) = delete;

public:
	std::pair<bool, _ClientKeyType> LogInToZone(ZoneContUnit* inClientContUnit, Zone*);
	void LogOutFromZone(SocketInfo*, ZoneContUnit* inClientContUnit);

private:
	void SendRemovePlayerInOuttedClientViewList(SocketInfo* pOutClient, ZoneContUnit* inClientCont);
	
private:
	//std::shared_mutex connectLock;
	concurrency::concurrent_queue<USHORT> uniqueKeyPool;
};