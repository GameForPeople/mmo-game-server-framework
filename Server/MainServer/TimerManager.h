#pragma once

#include "InHeaderDefine.hh"

struct TimerMemoryHead;

/*
	TimerUnit
		- 타이머에서 사용되는 메모리 단위입니다.
*/

enum class TIME : /*unsigned short*/ int
{
	MAX_TIME = 600,

	SECOND = 10,
	MINUTE = 600,

	PLAYER_MOVE = 5,
	SLIME_MOVE = 10,
	SLIME_ATTACK = 5,

	GOLEM_ATTACK = 20,
	DRAGON_MOVE = 20,
	DRAGON_ATTACK = 10,

	SELF_HEAL = 50,
	
	MONSTER_REVIVAL = 300,
	CHARACTER_REVIVAL = 30,

	// 쿨타임
	KNIGHT_SKILL_1 = 150,
	KNIGHT_SKILL_2 = 200,

	ARCHER_SKILL_1 = 100,
	ARCHER_SKILL_2 = 250,

	WITCH_SKILL_1 = 100,
	WITCH_SKILL_2 = 300,

	// Tick형 동작.
	ITEM_HP = 10,
	ITEM_MP = 10,

	// 이벤트 확인 주기
	CC_NODAMAGE = 50,
	CC_FAINT = 20,
	CC_FREEZE = 30,
	CC_ELECTRIC = 30,
	CC_BURN = 10,

	CC_GOLEM_FAINT = 15
	//KNIGHT_CC_NODAMAGE_COOLTIME = 50,
	//KNIGHT_CC_FAINT_COOLTIME = 20,
	//ARCHER_CC_FREEZE_COOLTIME = 30,
	//WITCH_CC_ELECTRIC_COOLTIME = 15
};

enum class TIMER_TYPE
{
	NONE_TYPE,
	NPC_MOVE,
	NPC_ATTACK,
	NPC_CHASE_LEFT,
	NPC_CHASE_RIGHT,
	NPC_CHASE_UP,
	NPC_CHASE_DOWN,

	SKILL_1_COOLTIME,
	SKILL_2_COOLTIME,

	SELF_HEAL,
	REVIVAL,

	CC_NODAMAGE,
	CC_FAINT,
	CC_FREEZE,
	CC_ELECTRIC,
	CC_BURN_3,
	CC_BURN_2,
	CC_BURN_1,

	ITEM_HP,
	ITEM_MP,

	PLAYER_MOVE,
	PLAYER_ATTACK,

	PUSH_OLD_KEY
};

struct TimerUnit
{
	TIMER_TYPE timerType;
	_KeyType objectKey;

public:
	TimerUnit();
	~TimerUnit();
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
	static DWORD WINAPI StartTimerThread();

	void AddTimerEvent(TimerUnit*, TIME);

	_NODISCARD TimerUnit* PopTimerUnit();
	void PushTimerUnit(TimerUnit*);

private:
	HANDLE hIOCP;
	static TimerManager* instance;

	TimerManager(HANDLE hIOCP);
	~TimerManager();

	std::atomic<int> nowTime;	// 10분의 1초를 단위로 사용합니다. 10 = 1초, 100 = 10초
	std::vector<concurrency::concurrent_queue<TimerUnit*>> timerCont;

	concurrency::concurrent_queue<TimerUnit*> timerMemoryPool;
	std::vector<std::array<TimerMemoryHead, 10>> timerMemoryHeadCont;

	BYTE postQueuedFunctionCallCount;
public:
	void SetPostQueuedFunctionCallCountAndTimerMemoryHeadCont(const BYTE inCallCount);
	concurrency::concurrent_queue<TimerUnit*>* GetTimerContWithIndex(const int inTimerContIndex);
};