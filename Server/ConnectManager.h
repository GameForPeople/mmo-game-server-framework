#pragma once

struct SocketInfo;
class Scene;

class ConnectManager
{
	using _ClientNode = std::pair<bool, SocketInfo*>;
public:
	ConnectManager() = default;
	~ConnectManager() = default;

	ConnectManager(const ConnectManager&) = delete;
	ConnectManager& operator=(const ConnectManager&) = delete;

public:
	_ClientNode InNewClient(std::vector< _ClientNode>& inClientCont, Scene*);
	void OutClient(SocketInfo*, std::vector< _ClientNode>& inClientCont);

private:
	void SendPutPlayer(SocketInfo* pPutClient, std::vector<_ClientNode>& inClientCont);
	void SendRemovePlayer(const char outClientKey, std::vector<_ClientNode>& inClientCont);
private:

	std::mutex connectLock;
};