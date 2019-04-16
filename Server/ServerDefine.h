#pragma once

/*
	ServerDefine.h
		- 해당 헤더 파일은, 서버에서만 사용합니다.
*/
struct SendMemoryUnit;
struct SocketInfo;
struct MemoryUnit;

namespace NETWORK_UTIL
{
	void SendPacket(SocketInfo* pClient, char* packetData);
	void RecvPacket(SocketInfo* pClient);
	void LogOutProcess(MemoryUnit* pClient);
	//_NODISCARD const bool GetRecvOrSendPacket(const LPVOID);
}

namespace BIT_CONVERTER
{
	constexpr BYTE SEND_BYTE = (1 << 7);

	/*_NODISCARD*/ BYTE MakeSendPacket(const BYTE inPacketType) noexcept;
	/*_NODISCARD*/ bool GetRecvOrSend(const char inChar) noexcept;
	
	/*_NODISCARD*/ BYTE MakeByteFromLeftAndRightByte(const BYTE inLeftByte, const BYTE inRightByte) noexcept;
	/*_NODISCARD*/ BYTE GetLeft4Bit(const BYTE inByte) noexcept;
	/*_NODISCARD*/ BYTE GetRight4Bit(const BYTE inByte) noexcept;
}

namespace ERROR_HANDLING {
	// 해당 static Function Array의 초기화는 GameServer의 생성자에서 이루어짐.
	static std::function<void(void)> errorRecvOrSendArr[2];
	inline void NotError(void) {};
	void HandleRecvOrSendError(void);

	_NORETURN void ERROR_QUIT(const CHAR *msg);
	/*_DEPRECATED*/ void ERROR_DISPLAY(const CHAR *msg);
}

namespace GLOBAL_DEFINE
{
	constexpr BYTE START_POSITION_X = 50;
	constexpr BYTE START_POSITION_Y = 50;

	constexpr BYTE SECTOR_DISTANCE = 10;	// 씐 전체 크기와 viewDistance를 고려해야함!
	constexpr BYTE SECTOR_HALF_DISTANCE = SECTOR_DISTANCE / 2;

	//---------
	constexpr BYTE MAX_CLIENT = 10;
	constexpr BYTE VIEW_DISTANCE = 4;
	constexpr USHORT MAX_SIZE_OF_RECV = 100;		//Recv 한번에 받을 수 있는 최대 사이즈
	constexpr USHORT MAX_SIZE_OF_RECV_PACKET = 2;//sizeof(PACKET_DATA::CLIENT_TO_SERVER::Down);	// (2) Recv 시, 처리해야하는 패킷 중 가장 큰 사이즈
	constexpr USHORT MAX_SIZE_OF_SEND = 5; // sizeof(PACKET_DATA::SERVER_TO_CLIENT::Position);	// (5) Send 시, 처리해야하는 패킷 중 가장 큰 사이즈
	constexpr USHORT MAX_NUMBER_OF_SEND_POOL = 100;
}