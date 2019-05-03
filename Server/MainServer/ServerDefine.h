#pragma once

/*
	ServerDefine.h
		- �ش� ��� ������, ���������� ����մϴ�.
*/
#define DISABLED_UNALLOCATED_MEMORY_SEND

struct SendMemoryUnit;
struct SocketInfo;
struct MemoryUnit;

namespace NETWORK_UTIL
{
	void SendPacket(SocketInfo* pClient, char* packetData);
#ifndef DISABLED_UNALLOCATED_MEMORY_SEND
	void SendUnallocatedPacket(SocketInfo* pClient, char* pOriginData);
#endif
	void RecvPacket(SocketInfo* pClient);
	void LogOutProcess(LPVOID pClient);
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
	// �ش� static Function Array�� �ʱ�ȭ�� GameServer�� �����ڿ��� �̷����.
	static std::function<void(void)> errorRecvOrSendArr[2];
	inline void NotError(void) {};
	void HandleRecvOrSendError(void);

	_NORETURN void ERROR_QUIT(const WCHAR *msg);
	/*_DEPRECATED*/ void ERROR_DISPLAY(const WCHAR *msg);
}

 namespace GLOBAL_DEFINE
{
	constexpr BYTE START_POSITION_X = 400;
	constexpr BYTE START_POSITION_Y = 400;

	constexpr BYTE SECTOR_DISTANCE = 20;	// �� ��ü ũ��� viewDistance�� �����ؾ���!
	constexpr BYTE SECTOR_HALF_DISTANCE = SECTOR_DISTANCE / 2;
	constexpr BYTE SECTOR_START_POSITION = 0;
	constexpr BYTE SECTOR_END_POSITION = SECTOR_DISTANCE - 1;

	constexpr BYTE VIEW_DISTANCE = 7;

	constexpr BYTE SECTOR_PLUS_LIMIT_DISTANCE = 2;	// ���� ũ��, �� ũ�� ���濡 ���� �缳���� �ʿ��մϴ�.
	constexpr BYTE SECTOR_MINUS_LIMIT_DISTANCE = 3;	// ���� ũ��, �� ũ�� ���濡 ���� �缳���� �ʿ��մϴ�.
	//---------
}