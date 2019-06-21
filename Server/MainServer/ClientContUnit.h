#pragma once

#include "InHeaderDefine.hh"

//#define _HASH_CONT
//#define _ASYNC_INIT

struct SocketInfo;
class BaseMonster;
struct BaseNpc;

struct ZoneContUnit
{
	std::array<SocketInfo*, GLOBAL_DEFINE::MAX_CLIENT> clientContArr;

	//std::array<std::atomic_flag, HASH_SIZE> lockArr;
	//std::array<std::atomic_int, HASH_SIZE> indexArr;
	//std::array<concurrency::concurrent_unordered_set<_ClientNode>, HASH_SIZE> clientCont;
	//tbb::concurrent_hash_map<_ClientNode> clientCont;
	//std::vector<_ClientNode> clientCont;

	std::array<BaseMonster*, GLOBAL_DEFINE::MAX_MONSTER> monsterCont;
	std::vector<BaseNpc*> npcCont;

public:
	ZoneContUnit();
	~ZoneContUnit();

	//void Enter(SocketInfo*);
	//void Exit(SocketInfo*);
	
	//std::pair<bool, SocketInfo*> FindClient(_ClientKeyType);
	//inline BYTE GetContHashKey(_ClientKeyType inClientKey) noexcept { return inClientKey % HASH_SIZE; }
	//inline BYTE GetContHashKey(WCHAR inFirstWChar) noexcept { return static_cast<BYTE>(inFirstWChar) % HASH_SIZE; }
};

struct SectorContUnit
{
	std::unordered_set<_ClientKeyType> clientCont;
	std::shared_mutex clientLock;

	std::unordered_set<_MonsterKeyType> monsterCont;
	std::shared_mutex monsterLock;
};


	//Concurrency::concurrent_unordered_set<_ClientKeyType> clientCont;
	//std::list<_ClientKeyType> clientCont;

struct UrbanSectorContUnit
{
};

struct RuralSectorContUnit
{
	Concurrency::concurrent_vector <std::pair<std::atomic<bool>, _ClientKeyType>> keyCont;
	std::mutex onlyWriteLock;
};