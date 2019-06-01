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
	//std::pair<bool, SocketInfo*> LogInToZone(ZoneContUnit* inClientContUnit, Zone*);
	//void LogOutToZone(SocketInfo*, ZoneContUnit* inClientContUnit);

	std::pair<bool, _ClientKeyType> GetUniqueKey();
	void PushOldKey(USHORT);

private:
	void SendRemovePlayerInOuttedClientViewList(SocketInfo* pOutClient, ZoneContUnit* inClientCont);
	
private:
	//std::shared_mutex connectLock;
	concurrency::concurrent_queue<USHORT> uniqueKeyPool;
public:
	std::pair<bool, SocketInfo*> OnlyGetUniqueKeyAndMallocSocketInfo();
};