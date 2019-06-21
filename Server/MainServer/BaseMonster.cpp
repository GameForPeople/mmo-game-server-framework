#include "pch.h"
#include "../Define.h"
#include "ServerDefine.h"

#include "ObjectInfo.h"
#include "BaseMonster.h"
#include "MonsterModelManager.h"
#include "LuaManager.h"

BaseMonster::BaseMonster(_KeyType inKey, _PosType inX, _PosType inY, const MonsterModel* const inMonsterModel)
	: objectInfo(nullptr)
	, key(inKey)
	, monsterModel(inMonsterModel)
	//, level()
	, spawnPosX(inX)
	, spawnPosY(inY)
	, isDead(false)
	, isSleep(false)
	//, noDamageTick(0)
	//, faintTick(0)
	, freezeTick(0)
	, electricTick(0)
	//, burnTick(0)
	, wakeUpClientKey(-1)
	, luaState(nullptr)
{
	objectInfo = new ObjectInfo(inX, inY);
	objectInfo->level = ( rand() % monsterModel->levelMaxDifference ) + monsterModel->startLevel;
	objectInfo->hp = monsterModel->hpPerLevel * objectInfo->level;
	objectInfo->damage = monsterModel->damagePerLevel * objectInfo->level;

	luaState = luaL_newstate();
	luaL_openlibs(luaState);	int error = luaL_loadfile(luaState, "LUA_Monster.lua") || lua_pcall(luaState, 0, 0, 0);
	if (error) LUA_UTIL::PrintError(luaState);

	if (monsterModel->monsterType == MONSTER_TYPE::SLIME)
	{
		objectInfo->job = JOB_TYPE::SLIME;

		lua_getglobal(luaState, "set_type");		lua_pushnumber(luaState, 1);
		lua_pcall(luaState, 1, 1, 0);
		lua_pop(luaState, 1);// eliminate set_uid from stack after call

	}
	else if (monsterModel->monsterType == MONSTER_TYPE::GOLEM)
	{
		objectInfo->job = JOB_TYPE::GOLEM;

		lua_getglobal(luaState, "set_type");		lua_pushnumber(luaState, 2);
		lua_pcall(luaState, 1, 1, 0);
		lua_pop(luaState, 1);// eliminate set_uid from stack after call
	}
	else if (monsterModel->monsterType == MONSTER_TYPE::DRAGON)
	{
		objectInfo->job = JOB_TYPE::DRAGON;

		lua_getglobal(luaState, "set_type");		lua_pushnumber(luaState, 3);
		lua_pcall(luaState, 1, 1, 0);
		lua_pop(luaState, 1);// eliminate set_uid from stack after call
	}

	lua_getglobal(luaState, "set_uid");	lua_pushnumber(luaState, inKey);
	lua_pcall(luaState, 1, 1, 0);
	lua_pop(luaState, 1);// eliminate set_uid from stack after call

	lua_register(luaState, "API_get_x", LuaManager::GetInstance()->API_get_x);
	lua_register(luaState, "API_get_y", LuaManager::GetInstance()->API_get_y);	lua_register(luaState, "API_get_IsLive", LuaManager::GetInstance()->API_get_IsLive);	lua_register(luaState, "API_go_SpawnPosition", LuaManager::GetInstance()->API_go_SpawnPosition);	lua_register(luaState, "API_do_attack", LuaManager::GetInstance()->API_do_attack);	lua_register(luaState, "API_do_chase", LuaManager::GetInstance()->API_do_chase);	lua_register(luaState, "API_Process", LuaManager::GetInstance()->API_Process);}

BaseMonster::~BaseMonster()
{
	// 몬스터 모델은 여기서 삭제되서는 안됩니다.
	// monsterModel = nullptr;
	lua_close(luaState);
	delete objectInfo;
}