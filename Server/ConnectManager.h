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
	_ClientNode InNewClient(ZoneContUnit* inClientContUnit, Zone*);
	void OutClient(SocketInfo*, ZoneContUnit* inClientContUnit);

private:
	void SendPutPlayer(SocketInfo* pPutClient, ZoneContUnit* inClientCont);
	void SendRemovePlayer(const char outClientKey, ZoneContUnit* inClientCont);
private:
	//std::shared_mutex connectLock;
};