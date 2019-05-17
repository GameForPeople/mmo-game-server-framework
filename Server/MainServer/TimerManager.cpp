#include "pch.h"

#include "ServerDefine.h"
#include "MemoryUnit.h"
#include "TimerManager.h"

TimerManager* TimerManager::instance = nullptr;

//---------------------------------------------------------------------------
// TimerUnit
//---------------------------------------------------------------------------
TimerUnit::TimerUnit()
	: timerType(TIMER_TYPE::NONE_TYPE), objectKey()
{
}

TimerUnit::~TimerUnit()
{
}

//---------------------------------------------------------------------------
// TimerManager
//---------------------------------------------------------------------------

TimerManager::TimerManager(HANDLE hIOCP) :
	hIOCP(hIOCP),
	nowTime(0), MAX_COOL_TIME(600 + 100),
	timerCont(),
	timerMemoryPool(),
	postQueuedFunctionCallCount(0)
{
	timerCont.reserve(MAX_COOL_TIME);
	for (int i = 0; i < MAX_COOL_TIME; ++i) { timerCont.emplace_back(); }

	for (int i = 0; i < 500000; ++i) { timerMemoryPool.push(new TimerUnit()); }
}

TimerManager::~TimerManager()
{
	TimerUnit* retTimerUnit = nullptr;
	while (timerMemoryPool.try_pop(retTimerUnit)) { delete retTimerUnit; }

	for (auto& iter : timerCont)
	{
		while (iter.try_pop(retTimerUnit)) { delete retTimerUnit; }
	}
}

void TimerManager::SetPostQueuedFunctionCallCountAndTimerMemoryHeadCont(const BYTE inCallCount)
{
	postQueuedFunctionCallCount = inCallCount;

	timerMemoryHeadCont.reserve(inCallCount);
	for (int i = 0; i < inCallCount; ++i) { timerMemoryHeadCont.emplace_back(); }
}

DWORD WINAPI TimerManager::StartTimerThread()
{
	TimerManager::GetInstance()->TimerThread();

	return 0;
};

void TimerManager::TimerThread()
{
	while (7)
	{
		Sleep(100); //0.1초 슬립인데 이부분 보정좀 해줘야해

		nowTime.load() != MAX_COOL_TIME - 1
			? nowTime.fetch_add(1)
			: nowTime = 0;

		const int tempInt = nowTime;
		const int tempArrIndex = tempInt % 10;

		for (int i = 0; i < postQueuedFunctionCallCount; ++i)
		{
			int errnum = PostQueuedCompletionStatus(hIOCP, 0, tempInt, reinterpret_cast<LPOVERLAPPED>(&timerMemoryHeadCont[i][tempArrIndex] /*+ sizeof(MEMORY_UNIT_TYPE)*/));
			if (errnum == 0)
			{
				errnum = WSAGetLastError();
				std::cout << errnum << std::endl;
				ERROR_HANDLING::ERROR_QUIT(L"PostQueuedCompletionStatus()");
			}
		}
		//TimerUnit* retTimerUnit = nullptr;
		//
		//while (timerCont[nowTime].try_pop(retTimerUnit))
		//{
		////----- 꺼낸 타이머 유닛 처리.
		//	int errnum = PostQueuedCompletionStatus(hIOCP, 0, retTimerUnit->objectKey, reinterpret_cast<LPOVERLAPPED>(retTimerUnit /*+ sizeof(MEMORY_UNIT_TYPE)*/));
		//	if (errnum == 0)
		//	{
		//		errnum = WSAGetLastError();
		//		std::cout << errnum << std::endl;
		//		ERROR_HANDLING::ERROR_QUIT(L"PostQueuedCompletionStatus()");
		//	}
		////-----
		//}
	}
}

void TimerManager::AddTimerEvent(TimerUnit* inTimerUnit, int waitTime)
{
	waitTime += nowTime;

	if (waitTime >= MAX_COOL_TIME) waitTime -= MAX_COOL_TIME;

	timerCont[waitTime].push(inTimerUnit);
}

TimerUnit* TimerManager::PopTimerUnit()
{
	TimerUnit* retTimerUnit = nullptr;
	
	return timerMemoryPool.try_pop(retTimerUnit)
		? retTimerUnit
		: []()->TimerUnit* { std::cout << "TimerUnit 이 부족해서 추가 할당을 하였습니다. " << std::endl; return new TimerUnit(); }();
}

void TimerManager::PushTimerUnit(TimerUnit* inTimerUnit)
{
	timerMemoryPool.push(inTimerUnit);
}

concurrency::concurrent_queue<TimerUnit*>*
TimerManager::GetTimerContWithIndex(const int inTimerContIndex)
{
	return &(timerCont[inTimerContIndex]);
}