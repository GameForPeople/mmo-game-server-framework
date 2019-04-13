#pragma once

#include "InHeaderDefine.hh"

struct SocketInfo;
struct ClientContUnit;
class Scene;

class ConnectManager
{
public:
	ConnectManager() = default;
	~ConnectManager() = default;

	ConnectManager(const ConnectManager&) = delete;
	ConnectManager& operator=(const ConnectManager&) = delete;

public:
	_ClientNode InNewClient(ClientContUnit* inClientContUnit, Scene*);
	void OutClient(SocketInfo*, ClientContUnit* inClientContUnit);

private:
	void SendPutPlayer(SocketInfo* pPutClient, ClientContUnit* inClientCont);
	void SendRemovePlayer(const char outClientKey, ClientContUnit* inClientCont);
private:
	//std::shared_mutex connectLock;
};