#include "pch.h"
#include "Define.h"
#include "ServerDefine.h"

#include "UserData.h"
#include "Zone.h"

#include "MemoryUnit.h"

//---------------------------------------------------------------------------
// MemoryUnit
//---------------------------------------------------------------------------

MemoryUnit::MemoryUnit(const bool InIsRecv) :
	overlapped(),
	wsaBuf(),
	dataBuf(nullptr),
	isRecv(InIsRecv)
{
#ifdef _DEV_MODE_
	std::cout << " MemoryUnit의 기본생성자가 호출되었습니다. \n";
#endif
	if (isRecv) dataBuf = new char[GLOBAL_DEFINE::MAX_SIZE_OF_RECV];
	else dataBuf = new char[GLOBAL_DEFINE::MAX_SIZE_OF_SEND];

	wsaBuf.buf = dataBuf;
}

MemoryUnit::~MemoryUnit()
{
#ifdef _DEV_MODE_
	std::cout << "그럴리가 없는데?? MemoryUnit의 소멸자가 호출되었습니다. \n";
#endif
	delete[] dataBuf;
}

MemoryUnit::MemoryUnit(const MemoryUnit& other) :
	overlapped(),
	wsaBuf(other.wsaBuf),
	dataBuf(other.dataBuf),
	isRecv(other.isRecv)
{
#ifdef _DEV_MODE_
	std::cout << " MemoryUnit의 복사생성자가 호출되었습니다. \n";
#endif
}

MemoryUnit::MemoryUnit(MemoryUnit&& other) noexcept :
	overlapped(),
	wsaBuf(),
	dataBuf(nullptr),
	isRecv(false)
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
		// 아니 원성연님, 생각을 해봐 이거 지워도 되는거 맞아요? ㅎㅎㅎ모르겠어욯ㅎㅎㅎㅎ
		delete[] dataBuf;

		dataBuf = other.dataBuf;
		wsaBuf = other.wsaBuf;
		other.dataBuf = nullptr;
	}
	return *this;
}

//---------------------------------------------------------------------------
// SendMemoryUnit
//---------------------------------------------------------------------------

SendMemoryUnit::SendMemoryUnit() 
	: memoryUnit(false)
	//, pOwner(nullptr)
{
}

SendMemoryUnit::~SendMemoryUnit()
{
}

SendMemoryUnit::SendMemoryUnit(const SendMemoryUnit& other) 
	: memoryUnit(other.memoryUnit)
	//, pOwner(other.pOwner)
{
#ifdef _DEV_MODE_
	std::cout << " SendMemoryUnit의 복사생성자가 호출되었습니다. \n";
#endif
}

SendMemoryUnit::SendMemoryUnit(SendMemoryUnit&& other) noexcept 
	: memoryUnit()
	//, pOwner(nullptr)
{
	*this = std::move(other);
}

SendMemoryUnit& SendMemoryUnit::operator=(SendMemoryUnit&& other) noexcept
{
#ifdef _DEV_MODE_
	std::cout << " SendMemoryUnit의 이동 할당 연산자(혹은 이동 생성자)가 호출되었습니다. \n";
#endif
	if (this != &other)
	{
		memoryUnit = std::move(other.memoryUnit);
		
		//pOwner = other.pOwner;
		//other.pOwner = nullptr;
	}
	return *this;
}

//---------------------------------------------------------------------------
// SocketInfo
//---------------------------------------------------------------------------
SocketInfo::SocketInfo() /*noexcept*/ :
	memoryUnit(true),
	sock(),
	loadedSize(),
	loadedBuf(),
	userData(new UserData(GLOBAL_DEFINE::START_POSITION_X, GLOBAL_DEFINE::START_POSITION_Y)/*std::make_unique<UserData>(0, 0)*/),
	clientKey(-1),
	pZone(nullptr),
	viewList(),
	sectorArr(),
	sectorIndexX(),
	sectorIndexY(),
	possibleSectorCount()
{
	loadedBuf = new char[GLOBAL_DEFINE::MAX_SIZE_OF_RECV_PACKET];
	viewList.clear();
}

SocketInfo::~SocketInfo()
{
	viewList.clear();

	delete[] loadedBuf;
	delete userData;
}