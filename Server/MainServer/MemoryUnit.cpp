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
	else
	{

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
	monsterViewList(),
	objectInfo(nullptr)
	//contIndex()
	//sectorArr(),
	//sectorIndexX(),
	//sectorIndexY(),
	//possibleSectorCount()
{
	viewList.clear();
	monsterViewList.clear();
	
	objectInfo = new PlayerObjectInfo();
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
	objectInfo->hp = 0;
	/*
		버퍼들은 초기화하지 않음.
	*/

	/*
		여기서 원래.. 뭔말인지 알지? 몬스터는 상관없는데, 클라 뷰리스트에는 상대방에게 알려줘야해!
	*/

	// 이미 다른데서 함.
	//viewListLock.lock();
	//viewList.clear();
	//viewListLock.unlock();
	//
	//monsterViewListLock.lock();
	//monsterViewList.clear();
	//monsterViewListLock.unlock();
	
	// 19 06 14 GameServer 개선.
	//delete objectInfo;	//
	//objectInfo = nullptr;	//
}

void SocketInfo::RegisterNewClient(SOCKET inSocket)
{
	sock = inSocket;
}

void SocketInfo::RegisterNewNickName(_NicknameType* inNewNick)
{
	memcpy(objectInfo->nickname, inNewNick, 20);
}

void SocketInfo::SetNewObjectInfo(_PosType x, _PosType y, _LevelType_T inlevel, _ExpType_T inExp, _JobType inJob,
	_HpType_T inHp, _MpType_T inMp, _MoneyType_T inMoney, _CountType_T inRedCount, _CountType_T inBlueCount, _TreeCountType inTreeCount)
{
	objectInfo->posX = x;
	objectInfo->posY = y;
	objectInfo->level = inlevel;
	objectInfo->exp = inExp;
	objectInfo->job = inJob;
	objectInfo->hp = inHp;
	objectInfo->mp = inMp;
	objectInfo->money = inMoney;
	objectInfo->redCount = inRedCount;
	objectInfo->blueCount = inBlueCount;
	objectInfo->treeCount = inTreeCount;

	objectInfo->noDamageFlag = false;
	objectInfo->selfHealFlag= false;
	objectInfo->moveFlag = true;
	objectInfo->attackFlag = true;
	objectInfo->skill1Flag = true;
	objectInfo->skill2Flag = true;
	objectInfo->redTickCount = 0;
	objectInfo->blueTickCount = 0;
}

void SocketInfo::CopyOtherObjectInfo(PlayerObjectInfo* otherObjectInfo)
{
	objectInfo->posX = otherObjectInfo->posX;
	objectInfo->posY = otherObjectInfo->posY;
	//objectInfo->level = otherObjectInfo->level;
	//objectInfo->exp = otherObjectInfo->exp;
	objectInfo->job = otherObjectInfo->job;
	//objectInfo->hp = otherObjectInfo->hp;
	//objectInfo->mp = otherObjectInfo->mp;
	//objectInfo->money = otherObjectInfo->money;
	//objectInfo->redCount = otherObjectInfo->redCount;
	//objectInfo->blueCount = otherObjectInfo->blueCount;
	objectInfo->treeCount = otherObjectInfo->treeCount;

	objectInfo->noDamageFlag = false;
	objectInfo->selfHealFlag = false;
	objectInfo->moveFlag = true;
	objectInfo->attackFlag = true;
	objectInfo->skill1Flag = false;
	objectInfo->skill2Flag = false;
	objectInfo->redTickCount = 0;
	objectInfo->blueTickCount = 0;
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