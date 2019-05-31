#pragma once

#define _USE_STD_FUNCTION_	false
//#define _USE_LAMBDA_ true

// 인라인하기 위해, 헤더에 인클루드함. 해당 헤더는 많이 가벼운편임.
#include "ObjectInfo.h"
//struct ObjectInfo;

struct SocketInfo;
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
	std::function<void(MoveManager&, ObjectInfo*)> whatIsYourDirection[static_cast<int>(DIRECTION::ENUM_SIZE)];
	std::function<void(MoveManager&, ObjectInfo*)> moveFunctionArr[static_cast<int>(DIRECTION::ENUM_SIZE)][2 /* Fail or Success */];
#else

#endif
	bool MoveCharacter(SocketInfo* pClient);
	bool MoveRandom(ObjectInfo* pClient);

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
	inline bool MoveLeft(ObjectInfo* const inUserData) noexcept
	{
		if (inUserData->posX == 0) return false;
		else 
		{
			--(inUserData->posX);
			return true;
		}
	};

	inline bool MoveUp(ObjectInfo* const inUserData) noexcept
	{
		if (inUserData->posY == 0) return false;
		else
		{
			--(inUserData->posY);
			return true;
		}
	};

	inline bool MoveRight(ObjectInfo* const inUserData) noexcept
	{
		if (inUserData->posX == GLOBAL_DEFINE::MAX_WIDTH - 1) return false;
		else
		{
			++(inUserData->posX);
			return true;
		}
	};

	inline bool MoveDown(ObjectInfo* const inUserData) noexcept
	{
		if (inUserData->posY == GLOBAL_DEFINE::MAX_HEIGHT - 1) return false;
		else
		{
			++(inUserData->posY);
			return true;
		}
	};
#endif
};