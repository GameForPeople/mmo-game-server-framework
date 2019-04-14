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
	// Sector로 이사갔습니다.
	//void SendPutPlayer(SocketInfo* pPutClient, ZoneContUnit* inClientCont);
	void SendRemovePlayer(SocketInfo* pOutClient, ZoneContUnit* inClientCont);

private:
	//std::shared_mutex connectLock;
};