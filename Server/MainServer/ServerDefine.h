#pragma once

/*
	ServerDefine.h
		- 해당 헤더 파일은, 서버에서만 사용합니다.
*/

#include "InHeaderDefine.hh"

#define DISABLED_UNALLOCATED_MEMORY_SEND

struct SendMemoryUnit;
struct SocketInfo;
struct MemoryUnit;
struct QueryMemoryUnit;
struct BaseMonster;

namespace NETWORK_UTIL
{
	//std::unique_ptr<QueryMemoryUnit> queryMemoryUnit;

	void SendPacket(SocketInfo* pClient, char* packetData);
	void SendQueryPacket(char* packetData);

#ifndef DISABLED_UNALLOCATED_MEMORY_SEND
	void SendUnallocatedPacket(SocketInfo* pClient, char* pOriginData);
#endif
	void RecvPacket(SocketInfo* pOutClient);
	void RecvQueryPacket();

	//void LogOutProcess(SocketInfo* pClient);
	//_NODISCARD const bool GetRecvOrSendPacket(const LPVOID);

	namespace SEND
	{
		template <class OBJECT, class PACKET_PUT> void SendPutPlayer(OBJECT* pPutObject, SocketInfo* pRecvClient);
		template <class OBJECT, class PACKET_POSITION> void SendMovePlayer(OBJECT* pMoveObject, SocketInfo* pRecvClient);
		void SendRemovePlayer(const _KeyType pRemoveObejctKey, SocketInfo* pRecvClient);
		void SendChatMessage(const WCHAR* message, const _KeyType, SocketInfo* pRecvClient);
		void SendStatChange(const char ENUM_STAT_CHANGE, const int inNewValue, SocketInfo* pRecvClient);
	}
}

namespace ITEM {
	constexpr UINT RED_PER_TICK = 20;
	constexpr UINT BLUE_PER_TICK = 10;
}

namespace DAMAGE
{
	constexpr float ELECTRIC_DAMAGE_NUMBER = 1.5;
}

namespace JOB {
	constexpr UINT MAX_LEVEL = 50;
	constexpr UINT MAX_EXP_PER_LEVEL = 100;

	constexpr UINT BASE_HP = 100;
	constexpr UINT KNIGHT_HP_PER_LEVEL = 10;
	constexpr UINT ARCHER_HP_PER_LEVEL = 5;
	constexpr UINT WITCH_HP_PER_LEVEL = 3;

	constexpr UINT BASE_MP = 50;
	constexpr UINT KNIGHT_MP_PER_LEVEL = 3;
	constexpr UINT ARCHER_MP_PER_LEVEL = 5;
	constexpr UINT WITCH_MP_PER_LEVEL = 10;

	constexpr UINT BASE_DAMAGE = 20;
	constexpr UINT KNIGHT_DAMAGE_PER_LEVEL = 10;
	constexpr UINT ARCHER_DAMAGE_PER_LEVEL = 5;
	constexpr UINT WITCH_DAMAGE_PER_LEVEL = 3;

	constexpr UINT KNIGHT_ATTACK_0_RANGE = 1;
	constexpr UINT KNIGHT_ATTACK_1_RANGE = 0;	// 공격스킬아님.
	constexpr UINT KNIGHT_ATTACK_2_RANGE = 3;

	constexpr UINT ARCHER_ATTACK_0_RANGE = 3;
	constexpr UINT ARCHER_ATTACK_1_RANGE = 3;	// 공격스킬아님.
	constexpr UINT ARCHER_ATTACK_2_RANGE = 3;

	constexpr UINT WITCH_ATTACK_0_RANGE = 2;
	constexpr UINT WITCH_ATTACK_1_RANGE = 6;	// 공격스킬아님.
	constexpr UINT WITCH_ATTACK_2_RANGE = 8;

	constexpr UINT MAX_TIMER_UNIT = 500000;

	unsigned short GetMaxHP(_JobType, unsigned char) noexcept;
	unsigned short GetMaxMP(_JobType, unsigned char) noexcept;
	_DamageType GetDamage(_JobType, unsigned char) noexcept;
	bool IsAttack(_JobType, unsigned char atkType, SocketInfo* pHitter, BaseMonster* pMonster) noexcept;
	bool/*private*/ IsInAttackRange(int range, SocketInfo* pHitter, BaseMonster* pMonster);
}

namespace ATOMIC_UTIL {
	template <class TYPE> bool T_CAS(std::atomic<TYPE> *addr, TYPE expected, TYPE new_val);
}

#include "ServerDefine.hpp"

namespace ERROR_HANDLING {
	_NORETURN void ERROR_QUIT(const WCHAR *msg);
	/*_DEPRECATED*/ void ERROR_DISPLAY(const WCHAR *msg);
	void HandleRecvError();
	bool HandleSendError();
}

namespace LUA_UTIL
{
	void PrintError(lua_State* L);
}

namespace TIME_UTIL
{
	const std::string GetCurrentDateTime();
}

 namespace GLOBAL_DEFINE
{
	constexpr USHORT MAX_CLIENT = 10000;
	constexpr UINT MAX_MONSTER = 10000;
	constexpr UINT MAX_TIMER_UNIT = 500000;

	constexpr USHORT START_POSITION_X = 165;
	constexpr USHORT START_POSITION_Y = 165;

	constexpr BYTE SECTOR_DISTANCE = 30;	// 씐 전체 크기와 viewDistance를 고려해야함!
	constexpr BYTE SECTOR_HALF_DISTANCE = SECTOR_DISTANCE / 2;
	constexpr BYTE SECTOR_START_POSITION = 0;
	constexpr BYTE SECTOR_END_POSITION = 9;

	constexpr BYTE VIEW_DISTANCE = 10;

	constexpr BYTE SECTOR_PLUS_LIMIT_DISTANCE = 4;	// 섹터 크기, 뷰 크기 변경에 따라 재설정이 필요합니다.
	constexpr BYTE SECTOR_MINUS_LIMIT_DISTANCE = 5;	// 섹터 크기, 뷰 크기 변경에 따라 재설정이 필요합니다.
	//---------
}