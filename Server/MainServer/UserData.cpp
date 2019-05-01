#include "pch.h"

#include "ServerDefine.h"

#include "UserData.h"

Position2D::Position2D(const UINT8 inPositionX = GLOBAL_DEFINE::START_POSITION_X, const UINT8 inPositionY = GLOBAL_DEFINE::START_POSITION_Y) noexcept
	: x(inPositionX)
	, y(inPositionY)
{}

UserData::UserData(const UINT8 inPositionX = 0, const UINT8 inPositionY = 0) noexcept
	: position(inPositionX, inPositionY)
{}
