#include "pch.h"
#include "../Define.h"
#include "ServerDefine.h"

#include "UserData.h"
#include "Zone.h"

#include "MemoryUnit.h"
#include "ObjectInfo.h"

//---------------------------------------------------------------------------
// MemoryUnit
//---------------------------------------------------------------------------

MemoryUnit::MemoryUnit(const MEMORY_UNIT_TYPE inMemoryUnitType) :
	overlapped(),
	wsaBuf(),
	memoryUnitType(inMemoryUnitType),
	dataBuf(nullptr)
{
#ifdef _DEV_MODE_
//	std::cout << " MemoryUnit의 기본생성자가 호출되었습니다. \n";
#endif
	if (MEMORY_UNIT_TYPE::RECV_FROM_CLIENT == inMemoryUnitType)
	{
		dataBuf = new char[GLOBAL_DEFINE::MAX_SIZE_OF_RECV];
		wsaBuf.len = GLOBAL_DEFINE::MAX_SIZE_OF_RECV;
	}
	else if (MEMORY_UNIT_TYPE::SEND_TO_CLIENT == inMemoryUnitType)
	{
		dataBuf = new char[GLOBAL_DEFINE::MAX_SIZE_OF_SEND];
	}
	else if (MEMORY_UNIT_TYPE::TIMER_FUNCTION == inMemoryUnitType)
	{
		wsaBuf.len = 0;
	}
	else if (MEMORY_UNIT_TYPE::RECV_FROM_QUERY == inMemoryUnitType)
	{
		dataBuf = new char[GLOBAL_DEFINE::MAX_SIZE_OF_RECV];
		wsaBuf.len = GLOBAL_DEFINE::MAX_SIZE_OF_RECV;
	}
	else if (MEMORY_UNIT_TYPE::RECV_FROM_COMMAND == inMemoryUnitType)
	{
		// 아직 사용되지 않음.
	}

	wsaBuf.buf = dataBuf;
}

MemoryUnit::~MemoryUnit()
{
#ifdef _DEV_MODE_
	//std::cout << "MemoryUnit의 소멸자가 호출되었습니다. \n";
#endif
	delete[] dataBuf;
}

MemoryUnit::MemoryUnit(const MemoryUnit& other) :
	overlapped(),
	wsaBuf(other.wsaBuf),
	dataBuf(other.dataBuf),
	memoryUnitType(other.memoryUnitType)
{
#ifdef _DEV_MODE_
	std::cout << " MemoryUnit의 복사생성자가 호출되었습니다. \n";
#endif
}

MemoryUnit::MemoryUnit(MemoryUnit&& other) noexcept :
	overlapped(),
	wsaBuf(),
	dataBuf(nullptr),
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
	: memoryUnit(MEMORY_UNIT_TYPE::SEND_TO_CLIENT)
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
	: memoryUnit(other.memoryUnit)
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
	}
	return *this;
}

//---------------------------------------------------------------------------
// SocketInfo
//---------------------------------------------------------------------------
SocketInfo::SocketInfo(_KeyType inKey) /*noexcept*/ :
	memoryUnit(MEMORY_UNIT_TYPE::RECV_FROM_CLIENT),
	sock(),
	loadedSize(),
	loadedBuf(),
	key(inKey),
	//posX(),
	//posY(),
	//userData(new UserData(GLOBAL_DEFINE::START_POSITION_X, GLOBAL_DEFINE::START_POSITION_Y)/*std::make_unique<UserData>(0, 0)*/),
	//clientKey(-1),
	//pZone(nullptr),
	viewList(),
	monsterViewList()
	//contIndex()
	//sectorArr(),
	//sectorIndexX(),
	//sectorIndexY(),
	//possibleSectorCount()
{
	viewList.clear();
	monsterViewList.clear();

	//objectInfo = new ObjectInfo(inKey, GLOBAL_DEFINE::START_POSITION_X, GLOBAL_DEFINE::START_POSITION_Y);
}

SocketInfo::~SocketInfo()
{
	viewList.clear();

	//delete[] loadedBuf;
	//delete userData;
}

void SocketInfo::TerminateClient()
{
	loadedSize = 0;
	/*
		버퍼들은 초기화하지 않음.
	*/

	/*
		여기서 원래.. 뭔말인지 알지? 몬스터는 상관없는데, 클라 뷰리스트에는 상대방에게 알려줘야해!
	*/
	viewList.clear();
	monsterViewList.clear();

	delete objectInfo;
	objectInfo = nullptr;
}

void SocketInfo::RegisterNewClient(SOCKET inSocket)
{
	sock = inSocket;
}

void SocketInfo::SetNewObjectInfo(PlayerObjectInfo* inNewClientObjectInfo)
{
	objectInfo = inNewClientObjectInfo;
}

//---------------------------------------------------------------------------
// TimerMemoryHead
//---------------------------------------------------------------------------

TimerMemoryHead::TimerMemoryHead(/*const unsigned short inTimerContIndex*/) noexcept
	: memoryUnit(MEMORY_UNIT_TYPE::TIMER_FUNCTION)/*, timerContIndex(inTimerContIndex)*/
{
}

//---------------------------------------------------------------------------
// QueryMemoryUnit
//---------------------------------------------------------------------------

QueryMemoryUnit::QueryMemoryUnit() noexcept :
	memoryUnit(MEMORY_UNIT_TYPE::RECV_FROM_QUERY),
	loadedBuf(),
	loadedSize()
{
}