#pragma once

#define DISABLED_FUNCTION_POINTER

struct UnallocatedMemoryUnit;
struct MemoryUnit;
struct SendMemoryUnit;
struct SocketInfo;
class Zone;

/*
	GameChatServer
		- 게임 채팅 서버입니다.
*/
class GameChatServer
{
public:
	void Run();

public:
	GameChatServer(bool);
	~GameChatServer();

	// 마 감히 서버가 어딜 복사하누!
	GameChatServer(const GameChatServer&) = delete;
	GameChatServer& operator=(const GameChatServer&) = delete;

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

#ifndef DISABLED_UNALLOCATED_MEMORY_SEND
	void AfterUnallocatedSend(UnallocatedMemoryUnit* pUnit);
#endif

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
