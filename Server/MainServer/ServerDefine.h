#pragma once

/*
	ServerDefine.h
		- 해당 헤더 파일은, 서버에서만 사용합니다.
*/
#define DISABLED_UNALLOCATED_MEMORY_SEND

struct SendMemoryUnit;
struct SocketInfo;
struct MemoryUnit;
struct QueryMemoryUnit;


namespace NETWORK_UTIL
{
	SOCKET querySocket;
	std::unique_ptr<QueryMemoryUnit> queryMemoryUnit;

	void SendPacket(SocketInfo* pClient, char* packetData);
	void SendQueryPacket(char* packetData);

#ifndef DISABLED_UNALLOCATED_MEMORY_SEND
	void SendUnallocatedPacket(SocketInfo* pClient, char* pOriginData);
#endif
	void RecvPacket(SocketInfo* pClient);
	void RecvQueryPacket();

	void LogOutProcess(LPVOID pClient);
	//_NODISCARD const bool GetRecvOrSendPacket(const LPVOID);
}

namespace ERROR_HANDLING {
	_NORETURN void ERROR_QUIT(const WCHAR *msg);
	/*_DEPRECATED*/ void ERROR_DISPLAY(const WCHAR *msg);
	void HandleRecvOrSendError();
}

 namespace GLOBAL_DEFINE
{
	constexpr USHORT MAX_CLIENT = 10000;
	constexpr UINT MAX_MONSTER = 200000;

	constexpr USHORT START_POSITION_X = 400;
	constexpr USHORT START_POSITION_Y = 400;

	constexpr BYTE SECTOR_DISTANCE = 20;	// 씐 전체 크기와 viewDistance를 고려해야함!
	constexpr BYTE SECTOR_HALF_DISTANCE = SECTOR_DISTANCE / 2;
	constexpr BYTE SECTOR_START_POSITION = 0;
	constexpr BYTE SECTOR_END_POSITION = 39;

	constexpr BYTE START_SECTOR_X = GLOBAL_DEFINE::START_POSITION_X / GLOBAL_DEFINE::SECTOR_DISTANCE;
	constexpr BYTE START_SECTOR_Y = GLOBAL_DEFINE::START_POSITION_Y / GLOBAL_DEFINE::SECTOR_DISTANCE;

	constexpr BYTE VIEW_DISTANCE = 7;

	constexpr BYTE SECTOR_PLUS_LIMIT_DISTANCE = 2;	// 섹터 크기, 뷰 크기 변경에 따라 재설정이 필요합니다.
	constexpr BYTE SECTOR_MINUS_LIMIT_DISTANCE = 3;	// 섹터 크기, 뷰 크기 변경에 따라 재설정이 필요합니다.
	//---------
}