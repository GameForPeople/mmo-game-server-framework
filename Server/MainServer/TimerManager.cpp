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

	SetPostQueuedFunctionCallCountAndTimerMemoryHeadCont(MAX_COOL_TIME);
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
	assert(timerMemoryHeadCont.size(), L"TimerThread가 실행되었으나, 워커쓰레드 갯수가 할당되지 않았습니다. SetPostQueuedFunctionCallCountAndTimerMemoryHeadCont를 호출해주세요.");

	while (7)
	{
		Sleep(100); //0.1초 슬립인데 이부분 보정좀 해줘야해

		nowTime.load() != MAX_COOL_TIME - 1
			? nowTime.fetch_add(1)
			: nowTime = 0;

		const int tempInt = nowTime;

		if (const int tempContSize = timerCont[tempInt].unsafe_size()
			; tempContSize == 0)
		{

		}
		else if (tempContSize < 10)
		{
			int errnum = PostQueuedCompletionStatus(hIOCP, 0, tempInt, reinterpret_cast<LPOVERLAPPED>(&timerMemoryHeadCont[0][tempInt % 10] /*+ sizeof(MEMORY_UNIT_TYPE)*/));
			if (errnum == 0)
			{
				errnum = WSAGetLastError();
				std::cout << errnum << std::endl;
				ERROR_HANDLING::ERROR_QUIT(L"PostQueuedCompletionStatus()");
			}
		}
		else
		{
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

// 성능적 단점이 있더라도, 실수 방지 위해, int -> TIME으로 변경.
void TimerManager::AddTimerEvent(TimerUnit* inTimerUnit, TIME waitTime)
{
	int tempWaitTime = static_cast<int>(waitTime);
	tempWaitTime += nowTime;

	if (tempWaitTime >= MAX_COOL_TIME) tempWaitTime -= MAX_COOL_TIME;

	timerCont[tempWaitTime].push(inTimerUnit);
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