#pragma once

class MoveManager;

struct SocketInfo;

class Scene
{
public:
	void ProcessRecvData(SocketInfo* pClient);

	Scene();
	~Scene() = default;
private:
	void InitManagers();
	void InitClientCont();
	void InitFunctions();

	void RecvCharacterMove(SocketInfo* pClient);

private:
	std::unique_ptr<MoveManager>				moveManager;
	std::function<void(Scene&, SocketInfo*)>	recvFunctionArr[PACKET_TYPE::ENUM_SIZE];
	std::array<std::pair<bool, SocketInfo*>, GLOBAL_DEFINE::MAX_CLIENT>	clientArr;
};