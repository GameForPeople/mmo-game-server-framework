#pragma once

#define _NOT_IF_MOVE_	// if가 아닌, 함수 포인터 배열을 활용해, Move.

class UserData;
struct SocketInfo;

enum class DIRECTION : BYTE
{
	LEFT /*= 0*/,
	UP /*= 1*/,
	RIGHT /*= 2*/,
	DOWN /*= 3*/,
	DIRECTION_END
};

class MoveManager
{
	static constexpr UINT8 BLOCK_MIN_POSITION = 0;
	static constexpr UINT8 BLOCK_MAX_POSITION = 7;

public:
	MoveManager() noexcept;
	~MoveManager() = default;

	MoveManager(const MoveManager&) = delete;
	MoveManager& operator=(const MoveManager&) = delete;

public:
#ifdef _NOT_IF_MOVE_
	std::function<void(MoveManager&, UserData*)> moveFunctions[static_cast<int>(DIRECTION::DIRECTION_END)][2 /* Fail or Success */];
#else
	std::function<void(MoveManager&, UserData* )> moveFunctions[static_cast<int>(DIRECTION::DIRECTION_END)];
#endif

	void MoveCharacter(SocketInfo* pClient);
	void SendMoveCharacter(SocketInfo* pClient);

private:
#ifdef _NOT_IF_MOVE_

	inline void MoveFail(UserData* inUserData) noexcept
	{}

	inline void MoveLeft(UserData* inUserData) noexcept
	{
		Position2D newPosition = inUserData->GetPosition();
		--newPosition.x;
		inUserData->SetPosition(newPosition);
	}

	inline void MoveUp(UserData* inUserData) noexcept
	{
		Position2D newPosition = inUserData->GetPosition();
		--newPosition.y;
		inUserData->SetPosition(newPosition);
	}

	inline void MoveRight(UserData* inUserData) noexcept
	{
		Position2D newPosition = inUserData->GetPosition();
		++newPosition.x;
		inUserData->SetPosition(newPosition);
	}

	inline void MoveDown(UserData* inUserData) noexcept
	{
		Position2D newPosition = inUserData->GetPosition();
		++newPosition.y;
		inUserData->SetPosition(newPosition);
	}

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