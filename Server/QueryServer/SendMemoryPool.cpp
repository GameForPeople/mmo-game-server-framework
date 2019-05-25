#include "pch.h"
#include "../Define.h"

#include "QueryDefine.h"
#include "MemoryUnit.h"
#include "SendMemoryPool.h"

SendMemoryPool* SendMemoryPool::instance = nullptr;

//---------------------------------------------------------------------------
// SendMemoryPool
//---------------------------------------------------------------------------

SendMemoryPool::SendMemoryPool()
{
	for (int i = 0; i < GLOBAL_DEFINE::MAX_NUMBER_OF_SEND_POOL; ++i)
		sendMemoryPool.push(new MemoryUnit(MEMORY_UNIT_TYPE::SEND) /* == std::move(SendMemoryUnit())*/);

#ifdef _DEV_MODE_
	std::cout << "sendMemoryPool의 사이즈는 " << sendMemoryPool.unsafe_size() << std::endl;
#endif
}

SendMemoryPool::~SendMemoryPool()
{
	MemoryUnit* tempUnit{ nullptr };
	while (sendMemoryPool.try_pop(tempUnit))
	{
		delete tempUnit;
	}
	delete SendMemoryPool::instance;
}

/*
	PopMemory()
		- Send에 필요한 MemoryUnit을 요청합니다.

	?0. 적절한 초기 메모리 풀 사이즈를 알기 위해, 현재 Send풀이 없을 경우, 로그를 출력합니다.
*/
MemoryUnit* SendMemoryPool::PopMemory()
{
	MemoryUnit* retMemoryUnit{ nullptr };

	// C6013 : NULL 포인터 'retMemoryUnit'을 역참조하고 있습니다.
	// 역참조되었지만 여전히 NULL 포인터 일 수 있습니다.
	while (!sendMemoryPool.try_pop(retMemoryUnit))
	{
		ERROR_HANDLING::ERROR_DISPLAY(L"[ERROR]SendPool의 메모리가 부족합니다.");
		/*
			원래는 여기서 메모리 추가로 할당해서, 넘겨줘야함. 기다리면 안됨.
		*/
	}
	return retMemoryUnit;
}

/*
	PushMemory()
		- Send가 끝난 메모리를 받아서 풀에 다시 집어넣습니다. 들어갔!

	?0. 사실 잘 들어가는지 한번 확인이 필요합니다.
*/
void SendMemoryPool::PushMemory(MemoryUnit* inMemoryUnit)
{
	sendMemoryPool.push(/*std::move(*/inMemoryUnit/*)*/);
}
