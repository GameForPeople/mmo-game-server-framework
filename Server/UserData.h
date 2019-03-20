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
public:
	UserData(const UINT8 inPositionX, const UINT8 inPositionY) noexcept;
	~UserData() = default;

	_NODISCARD inline Position2D GetPosition() const noexcept { return position; }
	inline void SetPosition(const Position2D& inNewPosition) noexcept { position = inNewPosition; }
	inline void SetPosition(const UINT8 inNewPositionX, const UINT8 inNewPositionY) noexcept 
	{
		position.x = inNewPositionX; position.y = inNewPositionY;
	}

private:
	Position2D position;
};