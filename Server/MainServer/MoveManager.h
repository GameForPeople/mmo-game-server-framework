#pragma once

#define _USE_STD_FUNCTION_	true
//#define _USE_LAMBDA_ true

struct SocketInfo;
class UserData;
struct ZoneContUnit;
struct ObjectInfo;
/*
	MoveManager
		- Zone 내부에서, 이동과 관련된 패킷을 처리하는 객체입니다.
*/
class MoveManager
{
public:
	MoveManager() noexcept;
	~MoveManager() = default;

	MoveManager(const MoveManager&) = delete;
	MoveManager& operator=(const MoveManager&) = delete;

public:
#if _USE_STD_FUNCTION_
	std::function<void(MoveManager&, ObjectInfo*)> whatIsYourDirection[static_cast<int>(DIRECTION::ENUM_SIZE)];
	std::function<void(MoveManager&, ObjectInfo*)> moveFunctionArr[static_cast<int>(DIRECTION::ENUM_SIZE)][2 /* Fail or Success */];
#else
	std::function<void(MoveManager&, UserData* )> moveFunctionArr[static_cast<int>(DIRECTION::DIRECTION_END)];
#endif
	void MoveCharacter(SocketInfo* pClient);
	void MoveRandom(ObjectInfo* pClient);

	//void SendMoveCharacter(SocketInfo* pMovedClient, ZoneContUnit* inClientCont);
private:
#if _USE_STD_FUNCTION_
	/*inline*/ void LeftMoveTest(ObjectInfo* );

	/*inline*/ void UpMoveTest(ObjectInfo* );

	/*inline*/ void RightMoveTest(ObjectInfo* );

	/*inline*/ void DownMoveTest(ObjectInfo* );

	/*inline*/ void MoveFail(ObjectInfo* ) noexcept;

	/*inline*/ void MoveLeft(ObjectInfo* ) noexcept;

	/*inline*/ void MoveUp(ObjectInfo* ) noexcept;

	/*inline*/ void MoveRight(ObjectInfo* ) noexcept;

	/*inline*/ void MoveDown(ObjectInfo* ) noexcept;

#else
	inline void MoveLeft(UserData* inUserData) noexcept
	{
		if (Position2D newPosition = inUserData->GetPosition()
			; newPosition.x == BLOCK_MIN_POSITION) return;
		else 
		{
			--newPosition.x;
			inUserData->SetPosition(newPosition);
		}
	};

	inline void MoveUp(UserData* inUserData) noexcept
	{
		if (Position2D newPosition = inUserData->GetPosition()
			; newPosition.y == BLOCK_MIN_POSITION) return;
		else
		{
			--newPosition.y;
			inUserData->SetPosition(newPosition);
		}
	};

	inline void MoveRight(UserData* inUserData) noexcept
	{
		if (Position2D newPosition = inUserData->GetPosition()
			; newPosition.x == BLOCK_MAX_POSITION) return;
		else
		{
			++newPosition.x;
			inUserData->SetPosition(newPosition);
		}
	};

	inline void MoveDown(UserData* inUserData) noexcept
	{
		if (Position2D newPosition = inUserData->GetPosition()
			; newPosition.y == BLOCK_MAX_POSITION) return;
		else
		{
			++newPosition.y;
			inUserData->SetPosition(newPosition);
		}
	};
#endif
};