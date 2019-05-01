#pragma once

struct MemoryUnit;
struct SendMemoryUnit;
struct SocketInfo;

class ZoneContUnit;
class ChatManager;

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
	void InitNetwork();

private:	// for Worker Thread
	static DWORD WINAPI StartWorkerThread(LPVOID arg);
	void WorkerThreadFunction();

private:	// for Aceept Thread
	static DWORD WINAPI StartAcceptThread(LPVOID arg);
	void AcceptThreadFunction();

private:
	void AfterRecv(SocketInfo* pClient, int cbTransferred);
	void AfterSend(SendMemoryUnit* pUnit);

	void ProcessRecvData(SocketInfo* pClient, int restSize);
	void ProcessPacket(SocketInfo* pClient);

	void ProcessChat(SocketInfo* pClient);
	void ProcessConnect(SocketInfo* pClient);
	void ProcessChange(SocketInfo* pClient);

private:
	WSADATA								wsa;
	HANDLE								hIOCP;
	SOCKET								listenSocket;

	SOCKADDR_IN							serverAddr;

	std::vector<std::thread>			workerThreadCont;

	ZoneContUnit*						zoneCont;	// 나중에는 존 개수에 따라 확장해야혀

	std::unique_ptr<ChatManager>		chatManager;	
};
