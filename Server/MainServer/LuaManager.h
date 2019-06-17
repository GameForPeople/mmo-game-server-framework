#pragma once

#include "InHeaderDefine.hh"

class Zone;

class LuaManager /**/
{
public:
	_NODISCARD static inline LuaManager* GetInstance() noexcept { return LuaManager::instance; };

	// 해당 함수는 GameServer.cpp의 생성자에서 한번 호출되어야합니다.
	/*_NODISCARD*/ static void MakeInstance(Zone* pZone) 
	{ LuaManager::instance = new LuaManager(pZone); /*return SendMemoryPool::instance;*/ };

	// 해당 함수는 GameServer.cpp의 소멸자에서 한번 호출되어야합니다.
	static void DeleteInstance() { delete instance; }

private:
	static LuaManager* instance;

	LuaManager(Zone*);
	~LuaManager() = default;

	LuaManager(const LuaManager&) = delete;
	LuaManager& operator=(const LuaManager&) = delete;

public:	//For Script Use
	Zone* pZone;

public: //Script function
	static int API_get_x(lua_State *L);
	static int API_get_y(lua_State *L);
	static int API_get_IsLive(lua_State *L);
	static int API_go_SpawnPosition(lua_State *L);
	static int API_do_attack(lua_State *L);
	static int API_do_chase(lua_State *L);
};