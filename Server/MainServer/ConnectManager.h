#pragma once

#include "InHeaderDefine.hh"

struct SocketInfo;
struct ZoneContUnit;

class Zone;

class ConnectManager /**/
{
public:
	_NODISCARD static inline ConnectManager* GetInstance() noexcept { return ConnectManager::instance; };

	// 해당 함수는 GameServer.cpp의 생성자에서 한번 호출되어야합니다.
	/*_NODISCARD*/ static void MakeInstance() { ConnectManager::instance = new ConnectManager(); /*return SendMemoryPool::instance;*/ };

	// 해당 함수는 GameServer.cpp의 소멸자에서 한번 호출되어야합니다.
	static void DeleteInstance() { delete instance; }

private:
	static ConnectManager* instance;

	ConnectManager();
	~ConnectManager() = default;

	ConnectManager(const ConnectManager&) = delete;
	ConnectManager& operator=(const ConnectManager&) = delete;

public:
	//std::pair<bool, SocketInfo*> LogInToZone(ZoneContUnit* inClientContUnit, Zone*);
	//void LogOutToZone(SocketInfo*, ZoneContUnit* inClientContUnit);

	std::pair<bool, _ClientKeyType> GetUniqueKey();
	void PushOldKey(USHORT);

private:
	void SendRemovePlayerInOuttedClientViewList(SocketInfo* pOutClient, ZoneContUnit* inClientCont);
	
private:
	//std::shared_mutex connectLock;
	concurrency::concurrent_queue<USHORT> uniqueKeyPool;
public:
	std::pair<bool, SocketInfo*> OnlyGetUniqueKeyAndMallocSocketInfo();
};