#pragma once

class WGameFramework;
struct MemoryUnit;

class NetworkManager
{
public:
	NetworkManager(const std::string_view&, WGameFramework*);
	~NetworkManager();

	void SendMoveData(const BYTE inDirection);
	void SendAttack(const unsigned char inAttackType);
	void SendItem(const unsigned char inItemType);

	void LogInOrSignUpProcess();
private:
	void InitNetwork();

	// for Worker Thread
	static DWORD WINAPI StartWorkerThread(LPVOID arg);
	void WorkerThreadFunction();

	// for Send Recv
	void SendPacket(char* packetData);
	void RecvPacket();

	void AfterRecv(/*MemoryUnit* pClient,*/ int cbTransferred);
	void AfterSend(MemoryUnit* pMemoryUnit);

	void ProcessRecvData(int restSize);
	void ProcessLoadedPacket();

private:
	std::string ipAddress;
	WGameFramework* pGameFramework;

	HANDLE hIOCP;
	std::thread workerThread;

	WSADATA wsa;
	SOCKET socket;
	SOCKADDR_IN serverAddr;

	//MemoryUnit sendMemoryUnit;
	MemoryUnit* recvMemoryUnit;

	char loadedBuf[GLOBAL_DEFINE::MAX_SIZE_OF_RECV];
	int loadedSize;

public:
};