#pragma once

#include "InHeaderDefine.hh"

enum class MEMORY_UNIT_TYPE : BYTE
{
	RECV_FROM_CLIENT = 0x00,
	SEND_TO_CLIENT = 0x01,

	TIMER_FUNCTION = 0x02,
	RECV_FROM_COMMAND = 0x03,
	//UNALLOCATED_SEND = 0x04
	ENUM_SIZE
};

#ifndef DISABLED_UNALLOCATED_MEMORY_SEND
/*
	UnallocatedMemoryUnit
		- 할당하지 않고, 공유 버퍼를 활용하여 Send하는 메모리에 사용됩니다.
*/
struct UnallocatedMemoryUnit
{
	OVERLAPPED overlapped;
	WSABUF wsaBuf;

	const MEMORY_UNIT_TYPE memoryUnitType;	// 해당 변수는 생성 시에 정의되고 변하지 않음.
};
#endif

/*
	MemoryUnit
		- 데이터 전송 Send, Recv 및, 해당 처리에서 필요한 목록을 저장해놓은 구조체입니다.
	
	#0. 해당 구조체의 용도는 (isRecv)를 통해 알 수 있습니다.

	#1. 해당 구조체는, 사용되는 목적에 따라,
		- Recv용은, SocketInfo에서 가장 상위에서 선언되는 정적 멤버 변수로 사용됩니다.
		- Send용은, SendMemoryUnit에서 가장 상위에서 선언되는 정적 멤버 변수로 사용됩니다.

	#2. dataBuf의 사이즈는, 사용되는 목적에 따라,
		- Recv용은, GLOBAL_DEFINE::MAX_SIZE_OF_RECV 만큼
		- Send용은, GLOBAL_DEFINE::MAX_SIZE_OF_SEND 만큼 할당받습니다.

	!0. #2와 관련하여, MAX_SIZE_OF_SEND보다 큰 경우에 대하여 전송을 요청할 경우, 오류가 발생합니다.
		- 따라서 항상, MAX_SIZE_OF_SEND는 전송되는 패킷들 중 가장 큰 사이즈로 정의되야 합니다.

	!1. 해당 구조체는 단독으로 사용되지 않습니다.

	!2. [MAIN] 19/05/03 이후로, 해당 구조체 멤버 변수의 순서가 변경되었습니다.
*/
struct MemoryUnit
{
	const MEMORY_UNIT_TYPE memoryUnitType;	// 해당 변수는 생성 시에 정의되고 변하지 않음.
	
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	
	char *dataBuf;

public:
	MemoryUnit(const MEMORY_UNIT_TYPE InMemoryUnitType);
	~MemoryUnit();

	MemoryUnit(const MemoryUnit& other);
	MemoryUnit(MemoryUnit&& other) noexcept;
	MemoryUnit& operator=(MemoryUnit&& other) noexcept;
};

/*
	SendMemoryUnit
		- Send 시, 사용되는 MemoryUnit입니다.

	#0. pOwner는 해당 Send에 대상이 되는 Socket Info 구조체입니다.
		- 이는 Send 실패 시, 예외처리를 위해 사용됩니다.

	!0. 멤버 변수 가장 상위에는 MemoryUnit가 있어야합니다. 절대로 보장되야합니다.

	?0. POwner의 쓰임에 의문이 생겻습니다. 이거 필요가 없는 걸?
		- 관련 코드 모드 주석 처리하고, 확정 시, 전부 삭제.
*/
//struct SocketInfo;

struct SendMemoryUnit
{
	MemoryUnit memoryUnit;
	//SocketInfo* pOwner;	// Send 예외처리 제외 -> 추후 삭제 확정 시 pOwner로 Search!

	SendMemoryUnit();
	~SendMemoryUnit();

	SendMemoryUnit(const SendMemoryUnit& other);
	SendMemoryUnit(SendMemoryUnit&& other) noexcept;
	SendMemoryUnit& operator=(SendMemoryUnit&& other) noexcept;
};

/*
	SocketInfo
		- 소켓정보구조체 입니다.

	!0. 멤버 변수 가장 상위에는 MemoryUnit가 있어야합니다. 절대로 보장되야합니다.
*/
class Zone;
/*
	4바이트 정렬 짓 해야합니다 여기.
*/

struct SocketInfo
{
public:
	SocketInfo() /*noexcept*/;
	~SocketInfo();

public:
	MemoryUnit memoryUnit;

	char loadedBuf[GLOBAL_DEFINE::MAX_SIZE_OF_RECV_PACKET];
	int loadedSize;

	SOCKET sock;
	std::wstring nickname;
	_ClientKeyType clientKey;
	BYTE contIndex;

	// UserData* userdata
	_PosType posX;	// 성능상의 이슈로 유저데이터 결국 분리. posX, posY 모두 socketInfo에 직접
	_PosType posY;

	Zone* pZone;		// 현재 입장한 존.

	BYTE sectorIndexX;	// 자신의 섹터로 슥~
	BYTE sectorIndexY;	// 자신의 섹터로 슥~

	BYTE possibleSectorCount;	// 검사해야하는 섹터 개수, 최대 3을 초과할 수 없음.
	std::array<std::pair<BYTE, BYTE>, 3> sectorArr;
	Concurrency::concurrent_unordered_set<_ClientKeyType> viewList;
};

/*
	TimerUnit
		- 타이머에서 사용되는 메모리 단위입니다.
*/

struct TimerUnit
{
	const MEMORY_UNIT_TYPE memoryUnitType;	// 해당 변수는 생성 시에 정의되고 변하지 않음.
	OBJECT_TYPE objectType;
	BYTE commandType;
	int objectKey;

public:
	TimerUnit();
	~TimerUnit();
};