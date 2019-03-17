#pragma once

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
	std::function<void(MoveManager&, UserData* )> moveFunctions[static_cast<int>(DIRECTION::DIRECTION_END)];
	void MoveCharacter(SocketInfo* pClient);
	void SendMoveCharacter(SocketInfo* pClient);
	
private:
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
};