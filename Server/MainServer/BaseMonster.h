#pragma once

#include "InHeaderDefine.hh"

struct ObjectInfo;
struct MonsterModel;

class BaseMonster {
public:
	BaseMonster(_KeyType, _PosType, _PosType, const MonsterModel* const);
	~BaseMonster();

public:
	ObjectInfo* objectInfo;
	const MonsterModel* monsterModel;
	lua_State* luaState;

	_KeyType key;

	_KeyType wakeUpClientKey;

	//_LevelType level;
	_PosType spawnPosX;
	_PosType spawnPosY;

	//_DamageType damage;
	_TickCountType freezeTick;
	_TickCountType electricTick;

	//_StateType state;
	/*std::atomic<bool>*/ bool isDead;
	std::atomic<bool> /*bool*/ isSleep;
};

namespace STATE
{
	enum
	{
		isDead = 1 << 0,
		isNoDamage = 1 << 1,
		isFaint = 1 << 2,
		isFreeze = 1 << 3,
		isElectric = 1 << 4,
	};

	inline constexpr bool IsDead(const _StateType inState) noexcept { return (inState & isDead); }
	inline constexpr bool IsNoDamage(const _StateType inState) noexcept { return (inState & isNoDamage); }
	inline constexpr bool IsFaint(const _StateType inState) noexcept { return (inState & isFaint); }
	inline constexpr bool IsFreeze(const _StateType inState) noexcept { return (inState & isFreeze); }
	inline constexpr bool IsElectric(const _StateType inState) noexcept { return (inState & isElectric); }

	inline constexpr void OnDead(_StateType& inState) noexcept { inState |= isDead; }
	inline constexpr void OnNoDamage(_StateType& inState) noexcept { inState |= isNoDamage; }
	inline constexpr void OnFaint(_StateType& inState) noexcept { inState |= isFaint; }
	inline constexpr void OnFreeze(_StateType& inState) noexcept { inState |= isFreeze; }
	inline constexpr void OnElectric(_StateType& inState) noexcept { inState |= isElectric; }

	inline constexpr void OffDead(_StateType& inState) noexcept { if(IsDead(inState)) inState ^= isDead; }
	inline constexpr void OffNoDamage(_StateType& inState) noexcept { if (IsNoDamage(inState)) inState ^= isNoDamage; }
	inline constexpr void OffFaint(_StateType& inState) noexcept { if (IsFaint(inState)) inState ^= isFaint; }
	inline constexpr void OffFreeze(_StateType& inState) noexcept { if (IsFreeze(inState)) inState ^= isFreeze; }
	inline constexpr void OffElectric(_StateType& inState) noexcept { if (IsElectric(inState)) inState ^= isElectric; }
}
