#pragma once

struct SocketInfo;

class GameServer
{
public:
	static constexpr USHORT	SERVER_PORT = 9000;
	constexpr inline USHORT GetServerPortNumber() { return SERVER_PORT; }

	void Run();

public:
	GameServer();
	~GameServer();

	GameServer(const GameServer& ) = delete;
	GameServer& operator=(const GameServer&) = delete;

private:	// for Init
	void InitForGameData();
	void InitForNetwork();

private:	// for Thread
	static DWORD WINAPI WorkerThread(LPVOID arg);
	void WorkerThreadFunction();

private:
	std::array <std::array 
		<std::function <void(GameServer&, SocketInfo*)>,
		static_cast<int>(PACKET_TYPE::PacketTypeCount)>, 
		2 /* Recv, Send */> functionArr;

	//void RecvFunction(SocketInfo* pClient);
	//void SendFunction(SocketInfo* pClient);
	
	void RecvCharacterMove(SocketInfo* pClient);
	void SendCharacterMove(SocketInfo* pClient);

private:
	WSADATA								wsa;
	HANDLE								hIOCP;
	SOCKET								listenSocket;

	SOCKADDR_IN							serverAddr;
};

#pragma region [Legacy Code]
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