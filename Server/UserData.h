#pragma once

struct Position2D 
{
	UINT8 x;
	UINT8 y;

	Position2D(const UINT8 inPositionX = 0, const UINT8 inPositionY = 0) noexcept
		: x(inPositionX), y(inPositionY)
	{}
};

class UserData 
{
	Position2D position;

public:
	UserData(const UINT8 inPositionX, const UINT8 inPositionY) noexcept;
	~UserData() = default;

public:
	//void MoveCharacter(const DIRECTION);
	
public:
	_NODISCARD inline Position2D GetPosition() const noexcept { return position; }
	inline void SetPosition(const Position2D& inNewPosition) noexcept { position = inNewPosition; }
};