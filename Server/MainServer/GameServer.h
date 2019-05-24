#pragma once

struct UnallocatedMemoryUnit;
struct MemoryUnit;

struct SendMemoryUnit;
struct SocketInfo;
struct TimerMemoryHead;

class Zone;

/*
	GameServer
		- 게임서버입니다.
		
		&0. Zone을 Vector로 여러개 관리하던 상황에서, 한 물리 서버에 한 개의 Zone만 을 관리하도록 변경(19/05/17)
			- 함수 : InitZones 
			- 변수 : zoneCont
		&1. 함수 배열을 통한 해쉬 기능에서, Switch-case로 변경, 불필요 제거(19/05/18)
			- 매크로 : DISABLED_FUNCTION_POINTER 
			- 함수 : InitFunctions
			- 변수 : recvOrSendArr
		&2. 쓰레드 함수의 리턴형 변경(19/05/18)
			- static DWORD WINAPI -> static void
		&3. 불필요한 함수 단계, 레거시로 설정 후, 직접 인라인. 
			- 함수명 : AfterSend, AfterRecv, ProcessTimerUnit
*/
class GameServer
{
public:
	void Run();

public:
	GameServer(bool);
	~GameServer();

	// 마 감히 서버가 어딜 복사하누!
	GameServer(const GameServer&) = delete;
	GameServer& operator=(const GameServer&) = delete;

private:	// for Init
	void ServerIntegrityCheck();
	void PrintServerInfoUI();
	void InitNetwork();

private:	// for Worker Thread
	static void StartWorkerThread(LPVOID arg);
	void WorkerThreadFunction();

private:	// for Aceept Thread
	static void StartAcceptThread(LPVOID arg);
	void AcceptThreadFunction();

private:
	void ProcessRecvData(SocketInfo* pClient, int restSize);


private:
	WSADATA								wsa;
	HANDLE								hIOCP;
	SOCKET								listenSocket;

	SOCKADDR_IN							serverAddr;

	std::vector<std::thread>			workerThreadCont;
	std::unique_ptr<Zone>				zone;
};

#pragma region [Legacy Code]
	//void AfterRecv(SocketInfo* pClient, int cbTransferred);
	//void AfterSend(SendMemoryUnit* pUnit);
	//void ProcessTimerUnit(const int timerManagerContIndex);
#pragma endregion