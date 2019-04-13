#pragma once

#include "InHeaderDefine.hh"

struct SocketInfo;
struct SceneContUnit;

class Scene;

class ConnectManager
{
public:
	ConnectManager() = default;
	~ConnectManager() = default;

	ConnectManager(const ConnectManager&) = delete;
	ConnectManager& operator=(const ConnectManager&) = delete;

public:
	_ClientNode InNewClient(SceneContUnit* inClientContUnit, Scene*);
	void OutClient(SocketInfo*, SceneContUnit* inClientContUnit);

private:
	void SendPutPlayer(SocketInfo* pPutClient, SceneContUnit* inClientCont);
	void SendRemovePlayer(const char outClientKey, SceneContUnit* inClientCont);
private:
	//std::shared_mutex connectLock;
};