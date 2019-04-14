#pragma once

#define _USE_STD_FUNCTION_	true
//#define _USE_LAMBDA_ true

struct SocketInfo;
class UserData;
struct ZoneContUnit;

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
	std::function<void(MoveManager&, UserData*)> whatIsYourDirection[static_cast<int>(DIRECTION::ENUM_SIZE)];
	std::function<void(MoveManager&, UserData*)> moveFunctionArr[static_cast<int>(DIRECTION::ENUM_SIZE)][2 /* Fail or Success */];
#else
	std::function<void(MoveManager&, UserData* )> moveFunctionArr[static_cast<int>(DIRECTION::DIRECTION_END)];
#endif
	void MoveCharacter(SocketInfo* pClient);
	void SendMoveCharacter(SocketInfo* pMovedClient, ZoneContUnit* inClientCont);
private:
#if _USE_STD_FUNCTION_
	/*inline*/ void LeftMoveTest(UserData* inUserData);

	/*inline*/ void UpMoveTest(UserData* inUserData);

	/*inline*/ void RightMoveTest(UserData* inUserData);

	/*inline*/ void DownMoveTest(UserData* inUserData);

	/*inline*/ void MoveFail(UserData* inUserData) noexcept;

	/*inline*/ void MoveLeft(UserData* inUserData) noexcept;

	/*inline*/ void MoveUp(UserData* inUserData) noexcept;

	/*inline*/ void MoveRight(UserData* inUserData) noexcept;

	/*inline*/ void MoveDown(UserData* inUserData) noexcept;

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