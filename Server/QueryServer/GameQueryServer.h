#pragma once
#include "InHeaderDefine.hh"

struct MemoryUnit;

/*
	GameQueryServer
		- 게임 DB 쿼리 서버입니다.
*/
class GameQueryServer
{
public:
	void Run();

public:
	GameQueryServer(bool);
	~GameQueryServer();

	// 마 감히 서버가 어딜 복사하누!
	GameQueryServer(const GameQueryServer&) = delete;
	GameQueryServer& operator=(const GameQueryServer&) = delete;

private:	// for Init
	void ServerIntegrityCheck();
	void PrintServerInfoUI(const std::string&);
	void InitNetwork(const std::string&);

private:	// for Worker Thread
	static void StartWorkerThread(LPVOID arg);
	void WorkerThreadFunction();

private:
	void AfterRecv(int cbTransferred);
	void AfterSend(MemoryUnit* pUnit);

	void ProcessRecvData(int restSize);
	void ProcessPacket();

	void SendPacket(char* packetData);
	void RecvPacket();

private:	// DB Functions
	void PrintDBErrorMessage(SQLHANDLE, SQLSMALLINT, RETCODE);
	bool InitAndConnectToDB();
	void FreeAndDisconnectDB();

	void ProcessDemandLogin();
	void ProcessDemandSignUp();
	void ProcessSaveLocation();
	void ProcessSaveUserInfo();

private:	// For Server
	WSADATA								wsa;
	HANDLE								hIOCP;
	SOCKET								socket;

	SOCKADDR_IN							mainServerAddr;
	SOCKADDR_IN							queryServerAddr;

	std::vector<std::thread>			workerThreadCont;

	MemoryUnit*							pRecvMemoryUnit;
	char								loadedBuf[GLOBAL_DEFINE::MAX_SIZE_OF_RECV_PACKET];
	int									loadedSize;

private:	// For DB
	SQLHENV henv;
	SQLHDBC hdbc;
	const SQLLEN intLen;
};
