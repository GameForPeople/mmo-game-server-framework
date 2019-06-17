#include "pch.h"
#include "../Define.h"
#include "ServerDefine.h"

#include "Zone.h"
#include "ClientContUnit.h"
#include "MemoryUnit.h"
#include "ObjectInfo.h"
#include "BaseMonster.h"

#include "LuaManager.h"

#include "MoveManager.h"
#include "TimerManager.h"
#include "MonsterModelManager.h"

LuaManager* LuaManager::instance = nullptr;

LuaManager::LuaManager(Zone* pZone)
	: pZone(pZone)
{}

/*_PosType*/ int LuaManager::API_get_x(lua_State *L)
{
	int user_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);

	int tempX;

	if (auto[objectType, realKey] = BIT_CONVERTER::WhatIsYourTypeAndRealKey(user_id);
		objectType == BIT_CONVERTER::OBJECT_TYPE::PLAYER)
	{
		tempX = LuaManager::GetInstance()->pZone->zoneContUnit->clientContArr[realKey]->objectInfo->posX;
	}
	else if (objectType == BIT_CONVERTER::OBJECT_TYPE::MONSTER)
	{
		tempX = LuaManager::GetInstance()->pZone->zoneContUnit->monsterCont[realKey]->objectInfo->posX;
	}

	lua_pushnumber(L, tempX);
	return 1;
}int LuaManager::API_get_y(lua_State *L)
{
	int user_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);

	int tempY;

	if (auto[objectType, realKey] = BIT_CONVERTER::WhatIsYourTypeAndRealKey(user_id);
		objectType == BIT_CONVERTER::OBJECT_TYPE::PLAYER)
	{
		tempY = LuaManager::GetInstance()->pZone->zoneContUnit->clientContArr[realKey]->objectInfo->posY;
	}
	else if (objectType == BIT_CONVERTER::OBJECT_TYPE::MONSTER)
	{
		tempY = LuaManager::GetInstance()->pZone->zoneContUnit->monsterCont[realKey]->objectInfo->posY;
	}

	lua_pushnumber(L, tempY);
	return 1;
}

int LuaManager::API_get_IsLive(lua_State *L)
{
	int user_id = (int)lua_tointeger(L, -2);
	_KeyType monsterKey = BIT_CONVERTER::WhatIsYourTypeAndRealKey((int)lua_tonumber(L, -1)).second;
	lua_pop(L, 3);

	int retResult;

	if (user_id == -1)
	{
		retResult = 0;
	}
	else if (LuaManager::GetInstance()->pZone->zoneContUnit->clientContArr[user_id]->objectInfo->hp == 0)
	{
		retResult = 0;
		LuaManager::GetInstance()->pZone->zoneContUnit->monsterCont[monsterKey]->wakeUpClientKey = -1;
	}
	else
	{
		retResult = 1;
	}

	lua_pushnumber(L, retResult);
	return 1;
}

int LuaManager::API_do_attack(lua_State *L)
{
	_KeyType playerKey = (int)lua_tonumber(L, -2);
	auto monsterKey = BIT_CONVERTER::WhatIsYourTypeAndRealKey((int)lua_tonumber(L, -1)).second;
	lua_pop(L, 3);
	
	auto pZoneContUnit = LuaManager::GetInstance()->pZone->zoneContUnit;
	// 전사 스킬 - 무적상태일 경우, 데미지 처리안함.
	if (pZoneContUnit->clientContArr[playerKey]->objectInfo->noDamageFlag) return 0;

	const int monsterDamage = pZoneContUnit->monsterCont[monsterKey]->objectInfo->damage;
	
	while (7)
	{
		unsigned short oldHp = pZoneContUnit->clientContArr[playerKey]->objectInfo->hp;
		
		if (oldHp > 0)
		{
			short newHp = oldHp - monsterDamage;
			if (newHp < 0) newHp = 0;

			if (pZoneContUnit->clientContArr[playerKey]->objectInfo->noDamageFlag) return 0;
			if (ATOMIC_UTIL::T_CAS(&(pZoneContUnit->clientContArr[playerKey]->objectInfo->hp)
				, oldHp, static_cast<unsigned short>(newHp)))
			{
				NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::HP, newHp, pZoneContUnit->clientContArr[playerKey]);

				if (newHp == 0)
				{
					while (7)
					{
						// 경험치 반토막
						unsigned int oldExp = pZoneContUnit->clientContArr[playerKey]->objectInfo->exp;
						unsigned int newExp = oldExp / 2;

						if (ATOMIC_UTIL::T_CAS(&(pZoneContUnit->clientContArr[playerKey]->objectInfo->exp)
							, oldExp, newExp))
						{
							NETWORK_UTIL::SEND::SendStatChange(STAT_CHANGE::EXP, newExp, pZoneContUnit->clientContArr[playerKey]);
							break;
						}
					}
					// Sector에서 내보내주고.
					LuaManager::GetInstance()->pZone->Death(pZoneContUnit->clientContArr[playerKey]);

					// 마 너 죽었어 임마!
					auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
					timerUnit->timerType = TIMER_TYPE::REVIVAL;
					timerUnit->objectKey = playerKey;
					TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::CHARACTER_REVIVAL);

					NETWORK_UTIL::SEND::SendRemovePlayer(playerKey, pZoneContUnit->clientContArr[playerKey]);
					NETWORK_UTIL::SEND::SendChatMessage(L"가 당신을 사망시켰습니다.", monsterKey, pZoneContUnit->clientContArr[playerKey]);
				}
				else
				{
					NETWORK_UTIL::SEND::SendChatMessage((L"가 당신에게 "+ std::to_wstring(monsterDamage) +L"의 데미지를 입혔습니다.").c_str(), monsterKey, pZoneContUnit->clientContArr[playerKey]);
					
					// 상태이상 설정
					if (pZoneContUnit->monsterCont[monsterKey]->monsterModel->monsterType == MONSTER_TYPE::GOLEM)
					{
						pZoneContUnit->clientContArr[playerKey]->objectInfo->faintTick.fetch_add(1);
						auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
						timerUnit->timerType = TIMER_TYPE::CC_FAINT;
						timerUnit->objectKey = playerKey;
						TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::CC_GOLEM_FAINT);
						NETWORK_UTIL::SEND::SendChatMessage(L"가 당신을 기절시켰습니다.", monsterKey, pZoneContUnit->clientContArr[playerKey]);
					}
					else if (pZoneContUnit->monsterCont[monsterKey]->monsterModel->monsterType == MONSTER_TYPE::DRAGON)
					{
						pZoneContUnit->clientContArr[playerKey]->objectInfo->burnTick.fetch_add(1);
						auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
						timerUnit->timerType = TIMER_TYPE::CC_BURN_3;
						timerUnit->objectKey = playerKey;
						TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::CC_BURN);
						NETWORK_UTIL::SEND::SendChatMessage(L"가 당신을 화상시켰습니다.", monsterKey, pZoneContUnit->clientContArr[playerKey]);
					}

					// 자힐 설정
					if (pZoneContUnit->clientContArr[playerKey]->objectInfo->selfHealFlag = true)
					{
						// 아무것도 안하는거 맞아요!
					}
					else
					{
						if (ATOMIC_UTIL::T_CAS(&(pZoneContUnit->clientContArr[playerKey]->objectInfo->selfHealFlag)
							, false, true))
						{
							auto timerUnit = TimerManager::GetInstance()->PopTimerUnit();
							timerUnit->timerType = TIMER_TYPE::SELF_HEAL;
							timerUnit->objectKey = playerKey;
							TimerManager::GetInstance()->AddTimerEvent(timerUnit, TIME::SELF_HEAL);
						}
					}
				}
				break;
			}
		}
	}
	return 0;
}

int LuaManager::API_do_chase(lua_State *L)
{
	_KeyType playerKey = (int)lua_tonumber(L, -2);
	_KeyType monsterKey = BIT_CONVERTER::WhatIsYourTypeAndRealKey((int)lua_tonumber(L, -1)).second;
	lua_pop(L, 3);

	// 나중에 해당 부분 변경 필요!!
	LuaManager::GetInstance()->pZone->moveManager
		->MoveRandom(LuaManager::GetInstance()->pZone->zoneContUnit->monsterCont[monsterKey]->objectInfo);

	return 0;
}

int LuaManager::API_go_SpawnPosition(lua_State *L)
{
		_KeyType playerKey = (int)lua_tonumber(L, -2);
	_KeyType monsterKey = BIT_CONVERTER::WhatIsYourTypeAndRealKey((int)lua_tonumber(L, -1)).second;
	lua_pop(L, 3);

	// 나중에 해당 부분 변경 필요!!
	LuaManager::GetInstance()->pZone->moveManager
		->MoveRandom(LuaManager::GetInstance()->pZone->zoneContUnit->monsterCont[monsterKey]->objectInfo);

	return 0;
}