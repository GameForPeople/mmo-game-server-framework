#pragma once

class UserData;

class MoveComponent
{
	static std::function<void(UserData&)> moveFunctions[static_cast<int>(DIRECTION::DIRECTION_END)];
	static constexpr UINT8 BLOCK_MIN_POSITION = 0;
	static constexpr UINT8 BLOCK_MAX_POSITION = 7;

public:
	MoveComponent() = default;
	~MoveComponent() = default;

public:
	inline void MoveLeft(UserData& inUserData) noexcept
	{
		if (Position2D refPosition = inUserData.RefPosition()
			; refPosition.x == BLOCK_MIN_POSITION) return;
		else --(refPosition.x);
	};

	inline void MoveUp(UserData& inUserData) noexcept
	{
		if (Position2D refPosition = inUserData.RefPosition()
			; refPosition.y == BLOCK_MIN_POSITION) return;
		else --(refPosition.y);
	};

	inline void MoveRight(UserData& inUserData) noexcept
	{
		if (Position2D refPosition = inUserData.RefPosition()
			; refPosition.x == BLOCK_MAX_POSITION) return;
		else ++(refPosition.x);
	};

	inline void MoveDown(UserData& inUserData) noexcept
	{
		if (Position2D refPosition = inUserData.RefPosition()
			; refPosition.y == BLOCK_MAX_POSITION) return;
		else ++(refPosition.y);
	};
};