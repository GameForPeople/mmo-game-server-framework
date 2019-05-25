#include "pch.h"
#include "../Define.h"

#include "MemoryUnit.h"

//---------------------------------------------------------------------------
// MemoryUnit
//---------------------------------------------------------------------------

MemoryUnit::MemoryUnit(const MEMORY_UNIT_TYPE inMemoryUnitType) :
	overlapped(),
	wsaBuf(),
	memoryUnitType(inMemoryUnitType),
	dataBuf()
{
#ifdef _DEV_MODE_
	std::cout << " MemoryUnit의 기본생성자가 호출되었습니다. \n";
#endif
	wsaBuf.len = GLOBAL_DEFINE::MAX_SIZE_OF_RECV_PACKET;
	wsaBuf.buf = dataBuf;
}

MemoryUnit::~MemoryUnit()
{
#ifdef _DEV_MODE_
	//std::cout << "MemoryUnit의 소멸자가 호출되었습니다. \n";
#endif
}

MemoryUnit::MemoryUnit(const MemoryUnit& other) :
	overlapped(),
	wsaBuf(other.wsaBuf),
	dataBuf(),
	memoryUnitType(other.memoryUnitType)
{
	memcpy(dataBuf, other.dataBuf, GLOBAL_DEFINE::MAX_SIZE_OF_RECV_PACKET);

#ifdef _DEV_MODE_
	std::cout << " MemoryUnit의 복사생성자가 호출되었습니다. \n";
#endif
}

MemoryUnit::MemoryUnit(MemoryUnit&& other) noexcept :
	overlapped(),
	wsaBuf(),
	dataBuf(),
	memoryUnitType(other.memoryUnitType)
{
	*this = std::move(other);
}

MemoryUnit& MemoryUnit::operator=(MemoryUnit&& other) noexcept
{
#ifdef _DEV_MODE_
	std::cout << " MemoryUnit의 이동 할당 연산자(혹은 이동 생성자)가 호출되었습니다. \n";
#endif
	if (this != &other)
	{
		memcpy(dataBuf, other.dataBuf, GLOBAL_DEFINE::MAX_SIZE_OF_RECV_PACKET);
		wsaBuf = std::move(other.wsaBuf);
	}
	return *this;
}
