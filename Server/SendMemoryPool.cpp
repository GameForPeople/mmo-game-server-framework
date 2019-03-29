#include "pch.h"
#include "Define.h"
#include "ServerDefine.h"

#include "SendMemoryPool.h"

SendMemoryPool* SendMemoryPool::instance = nullptr;

SendMemoryUnit::SendMemoryUnit() :
	overlapped(),
	wsaBuf(),
	sendBuf(nullptr)
{
#ifdef _DEV_MODE_
	std::cout << " SendMemoryPoolUnit의 기본생성자가 호출되었습니다. \n";
#endif
	sendBuf = new char[GLOBAL_DEFINE::MAX_SIZE_OF_SEND];
	wsaBuf.buf = sendBuf;
}

SendMemoryUnit::~SendMemoryUnit()
{
#ifdef _DEV_MODE_
	std::cout << "그럴리가 없는데?? SendMemoryPoolUnit의 소멸자가 호출되었습니다. \n";
#endif
	delete[] sendBuf;
}

SendMemoryUnit::SendMemoryUnit(const SendMemoryUnit& other)
	: overlapped(), wsaBuf(other.wsaBuf), sendBuf(other.sendBuf)
{
#ifdef _DEV_MODE_
	std::cout << " SendMemoryPoolUnit의 복사생성자가 호출되었습니다. \n";
#endif
}

SendMemoryUnit::SendMemoryUnit(SendMemoryUnit&& other) noexcept
	: overlapped(), wsaBuf(), sendBuf(nullptr)
{
	*this = std::move(other);
}

SendMemoryUnit& SendMemoryUnit::operator=(SendMemoryUnit&& other) noexcept
{
#ifdef _DEV_MODE_
	std::cout << " SendMemoryPoolUnit의 이동 할당 연산자(혹은 이동 생성자)가 호출되었습니다. \n";
#endif
	if (this != &other)
	{
		// 아니 원성연님, 생각을 해봐 이거 지워도 되는거 맞아요? ㅎㅎㅎ모르겠어욯ㅎㅎㅎㅎ
		delete[] sendBuf;
		sendBuf = other.sendBuf;
		wsaBuf = other.wsaBuf;
		other.sendBuf = nullptr;
	}
	return *this;
}

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
		- Send에 필요한 SendMemoryUnit을 요청합니다.

	?0. 적절한 초기 메모리 풀 사이즈를 알기 위해, 현재 Send풀이 없을 경우, 로그를 출력합니다.
*/
SendMemoryUnit* SendMemoryPool::PopMemory()
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

