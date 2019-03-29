#pragma once

/*
	SendMemoryUnit
		- 메모리풀에서 사용되는 단위 객체입니다.

	#0. sendBuf의 사이즈는 "GLOBAL_DEFINE::MAX_SIZE_OF_SEND"에서 결정합니다.

	!0. #0과 관련하여, MAX_SIZE_OF_SEND보다 큰 경우에 대하여 전송을 요청할 경우, 오류가 발생합니다.
		- 따라서 항상, MAX_SIZE_OF_SEND는 전송되는 패킷들 중 가장 큰 사이즈로 정의되야 합니다.
*/
struct SendMemoryUnit
{
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	char *sendBuf;

public:
	SendMemoryUnit();
	~SendMemoryUnit();

	SendMemoryUnit(const SendMemoryUnit& other);
	SendMemoryUnit(SendMemoryUnit&& other) noexcept;
	SendMemoryUnit& operator=(SendMemoryUnit&& other) noexcept;
};

/*
	SendMemoryPool
		- Send될 때 사용될 , 오버랩 구조체, wsaBuf, Buf를 미리 할당해 놓은 클래스입니다.

	!0. 해당 객체는 전역 - 싱글턴입니다.
		- 그 이유는 Send를 준비하는 위치가 다를 수 있기 때문입니다.
	
	!1. Send 요청이 많을 경우, 해당 Pool이 Empty 상태일 수 있습니다. 그럴 경우, 무한루프로 대기합니다.
		- 따라서 이를 방지하기 위해, 적절한 Pool 크기를 정해야합니다?

	?0. !1의 상황을 풀 사이즈를 바꾸는 것으로 해결하기 어려울 경우,
		풀이 가득 찼을 때, 동적으로 메모리를 할당받아 반환해주는 로직의 적용이 필요합니다.
		- 다만 현재는, 적절한 풀 사이즈를 알기 위하여, 제한합니다.
*/
class SendMemoryPool // Singleton
{
public:
	_NODISCARD static inline SendMemoryPool* GetInstance() noexcept { return SendMemoryPool::instance; };
	
	// 해당 함수는 GameServer.cpp의 생성자에서 한번 호출되어야합니다.
	/*_NODISCARD*/ static void MakeInstance() { SendMemoryPool::instance = new SendMemoryPool(); /*return SendMemoryPool::instance;*/ };
	
	// 해당 함수는 GameServer.cpp의 소멸자에서 한번 호출되어야합니다.
	static void DeleteInstance() { delete instance; }

	~SendMemoryPool();

	SendMemoryUnit* PopMemory();	// 메모리 제공
	void PushMemory(SendMemoryUnit*);	// 메모리 반납

private:
	static SendMemoryPool* instance;
	SendMemoryPool();

	Concurrency::concurrent_queue<SendMemoryUnit> sendMemoryPool;
};

