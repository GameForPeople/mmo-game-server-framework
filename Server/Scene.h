#pragma once

class MoveManager;
struct SocketInfo;

class Scene
{
public:
	void ProcessRecvData(SocketInfo* pClient);
	bool AddNewClient(SocketInfo* pClient);

	Scene();
	~Scene();
private:
	void InitManagers();
	void InitClientCont();
	void InitFunctions();

	void RecvCharacterMove(SocketInfo* pClient);

private:
	std::unique_ptr<MoveManager> moveManager;

	std::mutex addLock;
	std::function<void(Scene&, SocketInfo*)> recvFunctionArr[PACKET_TYPE::ENUM_SIZE];
	
	// 나중에 이부분 배열 들어내고, 리스트가 날 듯 보임.
	// 접속한 유저 리스트 + 남은 공간 큐
	std::array<std::pair<bool, SocketInfo*>, GLOBAL_DEFINE::MAX_CLIENT>	clientArr;
};