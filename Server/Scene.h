#pragma once

class MoveManager;
struct SocketInfo;
struct MemoryUnit;

/*
	Scene
		- GameServer(GameWorld)를 구성하는 단위 객체입니다.
	
	#0. 씐에 접속한 Client의 객체들의 컨테이너를 갖고, 관리합니다.
	#1. 네트워크 함수들은 GameServer의 함수가 아닌, 여기서 호출합니다.
	#2. 공간 분할의 단위, 기준입니다.
*/
class Scene
{
public:
	void ProcessPacket(SocketInfo* pClient);
	std::pair<bool, SocketInfo*> /*std::optional<SocketInfo*>*/ InNewClient();
	void OutClient(SocketInfo*);

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
	std::function<void(Scene&, SocketInfo*)>* recvFunctionArr;
	
	// 나중에 이부분 Array 들어내고, 리스트가 날 듯 보임.
	// 접속한 유저 리스트 + 남은 공간 큐
	std::vector<std::pair<bool, SocketInfo*>>	clientCont;
};