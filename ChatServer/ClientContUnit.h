#pragma once

#include "InHeaderDefine.hh"

struct SocketInfo;

class ZoneContUnit
{
	static constexpr BYTE HASH_SIZE = 10;

	std::array<std::vector<SocketInfo*>, HASH_SIZE> clientContArr;
	std::array<std::shared_mutex, HASH_SIZE> wrLockArr;

public:
	ZoneContUnit();
	~ZoneContUnit();

	void Enter(SocketInfo*);
	void Exit(SocketInfo*);

	inline BYTE GetContHashKey(WCHAR inKeyValue) noexcept
	{
		return static_cast<BYTE>(inKeyValue) % HASH_SIZE;
	}
};

