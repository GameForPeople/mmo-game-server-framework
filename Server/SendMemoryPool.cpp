#include "pch.h"
#include "Define.h"
#include "ServerDefine.h"

#include "MemoryUnit.h"
#include "SendMemoryPool.h"

SendMemoryPool* SendMemoryPool::instance = nullptr;

//---------------------------------------------------------------------------
// SendMemoryPool
//---------------------------------------------------------------------------

SendMemoryPool::SendMemoryPool()
{
	for(int i = 0 ; i < GLOBAL_DEFINE::MAX_NUMBER_OF_SEND_POOL; ++i)
		sendMemoryPool.push(std::move(SendMemoryUnit()));
}

SendMemoryPool::~SendMemoryPool()
{
	sendMemoryPool.clear();
	delete SendMemoryPool::instance;
}

/*
	PopMemory()
		- Send에 필요한 MemoryUnit을 요청합니다.

	?0. 적절한 초기 메모리 풀 사이즈를 알기 위해, 현재 Send풀이 없을 경우, 로그를 출력합니다.
*/
SendMemoryUnit* SendMemoryPool::PopMemory(SocketInfo* pClient)
{
	SendMemoryUnit* retMemoryUnit{nullptr};
	
	// C6013 : NULL 포인터 'retMemoryUnit'을 역참조하고 있습니다.
	// 역참조되었지만 여전히 NULL 포인터 일 수 있습니다.
	while (!sendMemoryPool.try_pop(*retMemoryUnit))
	{
		ERROR_HANDLING::ERROR_DISPLAY("[ERROR]SendPool의 메모리가 부족합니다.");
		/*
			원래는 여기서 메모리 추가로 할당해서, 넘겨줘야해 어딜 기다려!
		*/
	}

	//	retMemoryUnit->pOwner = pClient;
	return retMemoryUnit;
}

/*
	PushMemory()
		- Send가 끝난 메모리를 받아서 풀에 다시 집어넣습니다. 들어갔!

	?0. 사실 잘 들어가는지 한번 확인이 필요합니다.
*/
void SendMemoryPool::PushMemory(SendMemoryUnit* inMemoryUnit)
{
	sendMemoryPool.push(std::move(*inMemoryUnit));
}

