#include "pch.h"
#include "Define.h"
#include "ServerDefine.h"

#include "SendMemoryPool.h"

SendMemoryPool* SendMemoryPool::instance = nullptr;

SendMemoryPool::SendMemoryPool()
{
	for(int i = 0 ; i < GLOBAL_DEFINE::MAX_SIZE_OF_SEND_POOL; ++i)
		sendMemoryPool.push(std::move(SendMemoryUnit()));
}

SendMemoryPool::~SendMemoryPool()
{
	sendMemoryPool.clear();
	delete SendMemoryPool::instance;
}

void SendMemoryPool::PopMemory(SendMemoryUnit& outMemoryUnit)
{
	while (!sendMemoryPool.try_pop(outMemoryUnit))
	{
		GLOBAL_UTIL::ERROR_HANDLING::ERROR_DISPLAY("[ERROR]SendPool의 메모리가 부족합니다.");
	}
}

void SendMemoryPool::PushMemory(SendMemoryUnit&& inMemoryUnit)
{
	sendMemoryPool.push(std::move(inMemoryUnit));
}

