#pragma once

#include "NetworkType.h"	// only Enum
#include "PacketType.h"		// only Enum

struct SocketInfo;
class WorldManager;

// 싱글턴 적용.
class GameServer
{
public:
	static constexpr USHORT	SERVER_PORT = 9000;
	static constexpr BYTE SEND_BYTE = ( 1 << 7 );
	static constexpr USHORT	MAX_CLIENT_COUNT = 10;
	
public:
	static GameServer* instance;
	_NODISCARD static GameServer* GetInstance() noexcept { return GameServer::instance; } ;
	_NODISCARD static GameServer* MakeInstance() { GameServer::instance = new GameServer(); return GameServer::instance; };
	static void DestroyInstance() { delete GameServer::instance; };

	~GameServer();
	
	void Run();

private:
	GameServer();
	//GameServer(const GameServer&) = delete;
	//GameServer& operator=(const GameServer&) = delete;

	void PrintServerInfoUI() const;
	void InitManagers();
	void InitFunctions();
	void InitNetwork();

	static void CALLBACK CallBackRecv(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
	static void CALLBACK CallBackSend(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);

	void CallBackRecvProcess(DWORD dataBytes, LPWSAOVERLAPPED overlapped);
	void CallBackSendProcess(DWORD dataBytes, LPWSAOVERLAPPED overlapped);

	std::function <void(GameServer&, SocketInfo*)> recvFunctionArr[PACKET_TYPE::ENUM_SIZE];

	void RecvVoidUpdate(SocketInfo* pClient);
	void RecvCharacterMove(SocketInfo* pClient);

	inline /*int*/ bool GetRecvOrSend(const char inChar) noexcept { return (inChar >> 7) & (0x01); }
	inline char MakeSendPacket(const BYTE inPacketType) noexcept { return inPacketType | SEND_BYTE; }

	void LoadOtherPlayerPositionToSendBuffer(SocketInfo* pClient, const int inStartIndex);

private:
	WSADATA								wsa;
	SOCKET								listenSocket;

	SOCKADDR_IN							serverAddr;

	std::unique_ptr<WorldManager>		worldManager;

	std::map <SOCKET, SocketInfo*>		clientCont;
};