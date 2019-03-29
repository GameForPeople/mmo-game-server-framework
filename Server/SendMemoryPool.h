#pragma once

struct SendMemoryUnit
{
	WSABUF wsaBuf;
	char *buf;

public:
	SendMemoryUnit()
	{
#ifdef _DEV_MODE_
		std::cout << " SendMemoryPoolUnit의 기본생성자가 호출되었습니다. \n";
#endif
		buf = new char[GLOBAL_DEFINE::MAX_SIZE_OF_BUFFER];
		wsaBuf.buf = buf;
	}
	~SendMemoryUnit() {
#ifdef _DEV_MODE_
		std::cout << " SendMemoryPoolUnit의 소멸자가 호출되었습니다. \n";
#endif
		delete[] buf; 
	}

	SendMemoryUnit(const SendMemoryUnit& other)
		: wsaBuf(other.wsaBuf), buf(other.buf)
	{
#ifdef _DEV_MODE_
		std::cout << " SendMemoryPoolUnit의 복사생성자가 호출되었습니다. \n";
#endif
	}

	SendMemoryUnit(SendMemoryUnit&& other)
		: wsaBuf(), buf(nullptr)
	{
		*this = std::move(other);
	}

	SendMemoryUnit& operator=(SendMemoryUnit&& other)
	{
#ifdef _DEV_MODE_
		std::cout << " SendMemoryPoolUnit의 이동 할당 연산자(혹은 이동 생성자)가 호출되었습니다. \n";
#endif
		if (this != &other)
		{
			delete[] buf;
			buf = other.buf;
			wsaBuf = other.wsaBuf;
			other.buf = nullptr;
		}
		return *this;
	}
};

class SendMemoryPool // Singleton
{
public:
	_NODISCARD static inline SendMemoryPool* GetInstance() noexcept { return SendMemoryPool::instance; };
	
	// 해당 함수는 GameServer.cpp의 생성자에서 한번 호출되어야합니다.
	/*_NODISCARD*/ static void MakeInstance() { SendMemoryPool::instance = new SendMemoryPool(); /*return SendMemoryPool::instance;*/ };
	
	~SendMemoryPool();

	void PopMemory(SendMemoryUnit&);	// 메모리 제공
	void PushMemory(SendMemoryUnit&&);	// 메모리 반납

private:
	static SendMemoryPool* instance;
	SendMemoryPool();

	Concurrency::concurrent_queue<SendMemoryUnit> sendMemoryPool;
};

