#pragma once

#define DISABLED_FUNCTION_POINTER

struct UnallocatedMemoryUnit;
struct MemoryUnit;
struct SendMemoryUnit;
struct SocketInfo;
class Zone;

/*
	GameServer
		- 게임서버입니다.
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
	void InitZones();
	void InitFunctions();
	void InitNetwork();

private:	// for Worker Thread
	static DWORD WINAPI StartWorkerThread(LPVOID arg);
	void WorkerThreadFunction();

private:	// for Aceept Thread
	static DWORD WINAPI StartAcceptThread(LPVOID arg);
	void AcceptThreadFunction();

private:
#ifdef DISABLED_FUNCTION_POINTER
	void AfterRecv(SocketInfo* pClient, int cbTransferred);
	void AfterSend(SendMemoryUnit* pUnit);
	void AfterUnallocatedSend(UnallocatedMemoryUnit* pUnit);

#else
	std::function <void(GameServer&, LPVOID)>* recvOrSendArr;
	void AfterRecv(LPVOID pClient);
	void AfterSend(LPVOID pClient);
#endif
	void ProcessRecvData(SocketInfo* pClient, int restSize);

private:
	WSADATA								wsa;
	HANDLE								hIOCP;
	SOCKET								listenSocket;

	SOCKADDR_IN							serverAddr;

	std::vector<std::thread>			workerThreadCont;
	std::vector<std::unique_ptr<Zone>>	zoneCont;
};

#pragma region [Legacy]
/*

struct ServerInitInfomation 
{
	enum class SERVER_IP_TYPE : BYTE
	{
		LOCAL_HOST = 1
		, INPUTTED_IP = 2
		//,	AWS_PUBLIC = 3
	};

	SERVER_IP_TYPE serverIPType;
	std::string IPAddress;

public:
	ServerInitInfomation(const SERVER_IP_TYPE inServerIpType, const std::string_view& inIPAddress) noexcept
		: serverIPType(inServerIpType)
		, IPAddress(inIPAddress)
	{};

	~ServerInitInfomation() = default;
};
*/
#pragma endregion