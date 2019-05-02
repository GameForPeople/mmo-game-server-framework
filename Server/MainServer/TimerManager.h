#pragma once

#include "InHeaderDefine.hh"

struct TimerUnit
{
	_KeyType objectKey;
	BYTE commandType;
	OBJECT_TYPE objectType;
};

class TimerManager /*: 싱글턴 */
{
	const int MAX_COOL_TIME;

public:
	_NODISCARD static inline TimerManager* GetInstance() noexcept { return TimerManager::instance; };

	// 해당 함수는 GameServer.cpp의 생성자에서 한번 호출되어야합니다.
	/*_NODISCARD*/ static void MakeInstance(HANDLE hIOCP) { TimerManager::instance = new TimerManager(hIOCP); /*return SendMemoryPool::instance;*/ };

	// 해당 함수는 GameServer.cpp의 소멸자에서 한번 호출되어야합니다.
	static void DeleteInstance() { delete instance; }

public:
	void TimerThread();
	void AddTimerEvent(TimerUnit*, int);

	_NODISCARD TimerUnit* GetTimerUnit();

private:
	HANDLE hIOCP;
	static TimerManager* instance;
	TimerManager(HANDLE hIOCP);
	~TimerManager();

	std::atomic<int> nowTime;	// 10분의 1초를 단위로 사용합니다. 10 = 1초, 100 = 10초
	std::vector<concurrency::concurrent_queue<TimerUnit*>> timerCont;
	concurrency::concurrent_queue<TimerUnit*> timerMemoryPool;
};