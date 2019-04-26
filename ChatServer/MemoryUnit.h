#pragma once

#include "InHeaderDefine.hh"

enum class MEMORY_UNIT_TYPE : BYTE
{
	RECV = 0x00,
	SEND = 0x01,
	//UNALLOCATED_SEND = 0x02
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
*/
struct MemoryUnit
{
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	
	const MEMORY_UNIT_TYPE memoryUnitType;	// 해당 변수는 생성 시에 정의되고 변하지 않음.
	
	char dataBuf[GLOBAL_DEFINE::MAX_SIZE_OF_RECV_PACKET];

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
*/
//struct SocketInfo;

struct SendMemoryUnit
{
	MemoryUnit memoryUnit;

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

struct SocketInfo
{
public:
	SocketInfo() /*noexcept*/;
	~SocketInfo();

public:
	MemoryUnit memoryUnit;

	int loadedSize;
	char loadedBuf[GLOBAL_DEFINE::MAX_SIZE_OF_RECV_PACKET];

	SOCKET sock;
	
	BYTE zoneIndex;
	std::wstring nickname;
	USHORT contIndex;
};