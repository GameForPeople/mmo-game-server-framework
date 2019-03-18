#pragma once

#include "NetworkType.h"	// only Enum
#include "PacketType.h"		// only Enum

struct SocketInfo;
class MoveManager;

class GameServer
{
public:
	static constexpr USHORT	SERVER_PORT = 9000;
	static constexpr BYTE SEND_BYTE = ( 1 << 7 );

	void Run();

public:
	GameServer(bool);
	~GameServer();

	GameServer(const GameServer&) = delete;
	GameServer& operator=(const GameServer&) = delete;

private:	// for Init
	void PrintServerInfoUI();
	void InitManagers();
	void InitFunctions();
	void InitNetwork();

private:	// for Thread
	static DWORD WINAPI StartWorkerThread(LPVOID arg);
	void WorkerThreadFunction();

private:	// about Hash-Function
	std::function <void(GameServer&, SocketInfo*)> recvOrSendArr[NETWORK_TYPE::ENUM_SIZE];
	std::function <void(GameServer&, SocketInfo*)> recvFunctionArr[PACKET_TYPE::ENUM_SIZE];

	void AfterRecv(SocketInfo* pClient);
	void AfterSend(SocketInfo* pClient);
	
	void RecvCharacterMove(SocketInfo* pClient);

private:
	WSADATA								wsa;
	HANDLE								hIOCP;
	SOCKET								listenSocket;

	SOCKADDR_IN							serverAddr;

	std::unique_ptr<MoveManager>		moveManager;

private:	// Bit Converter
	inline /*int*/ bool GetRecvOrSend(const char inChar) noexcept { return (inChar >> 7) & (0x01); }
	//inline int GetPacketType(const char inChar) noexcept { return (inChar >> 7) & (0xfe); }

	inline char MakeSendPacket(const BYTE inPacketType) noexcept { return inPacketType | SEND_BYTE; }
	//inline char MakeSendPacket(const PACKET_TYPE inPacketType) noexcept { return static_cast<BYTE>(inPacketType) | SEND_BYTE; }
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