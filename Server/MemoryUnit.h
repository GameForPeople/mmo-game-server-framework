#pragma once

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
	
	const bool isRecv;	// 해당 변수는 생성 시에 정의되고 변하지 않음.
	
	char *dataBuf;

public:
	MemoryUnit(const bool InIsRecv = false);
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
class UserData;
class Scene;

struct SocketInfo
{
public:
	SocketInfo() /*noexcept*/;
	~SocketInfo();

public:
	MemoryUnit memoryUnit;

	int loadedSize;
	char *loadedBuf;

	SOCKET sock;
	UserData* userData;
	//std::unique_ptr<UserData> userData;

	int clientContIndex;
	Scene* pScene;
};