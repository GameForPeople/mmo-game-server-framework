#include "pch.h"

#include "UserData.h"
#include "SocketInfo.h"

#include "WorldManager.h"

WorldManager::WorldManager() noexcept
{
#if _USE_STD_FUNCTION_
	whatIsYourDirection[DIRECTION::LEFT] = &WorldManager::LeftMoveTest;
	whatIsYourDirection[DIRECTION::UP] = &WorldManager::UpMoveTest;
	whatIsYourDirection[DIRECTION::RIGHT] = &WorldManager::RightMoveTest;
	whatIsYourDirection[DIRECTION::DOWN] = &WorldManager::DownMoveTest;

	WorldManager::moveFunctionArr[DIRECTION::LEFT][0] = &WorldManager::MoveFail;//&WorldManager::MoveFail;
	WorldManager::moveFunctionArr[DIRECTION::LEFT][1] = &WorldManager::MoveLeft;
	WorldManager::moveFunctionArr[DIRECTION::UP][0] = &WorldManager::MoveFail;
	WorldManager::moveFunctionArr[DIRECTION::UP][1] = &WorldManager::MoveUp;
	WorldManager::moveFunctionArr[DIRECTION::RIGHT][0] = &WorldManager::MoveFail;
	WorldManager::moveFunctionArr[DIRECTION::RIGHT][1] = &WorldManager::MoveRight;
	WorldManager::moveFunctionArr[DIRECTION::DOWN][0] = &WorldManager::MoveFail;
	WorldManager::moveFunctionArr[DIRECTION::DOWN][1] = &WorldManager::MoveDown;
#else
	WorldManager::moveFunctionArr[DIRECTION::LEFT] = &WorldManager::MoveLeft;
	WorldManager::moveFunctionArr[DIRECTION::UP] = &WorldManager::MoveUp;
	WorldManager::moveFunctionArr[DIRECTION::RIGHT] = &WorldManager::MoveRight;
	WorldManager::moveFunctionArr[DIRECTION::DOWN] = &WorldManager::MoveDown;
#endif
}

void WorldManager::MoveCharacter(SocketInfo* pClient)
{
#ifdef _DEV_MODE_
	std::cout << "움직이는 방향은 : " << int(static_cast<int>(pClient->buf[1])) << "\n";
#endif

#if _USE_STD_FUNCTION_
	whatIsYourDirection[static_cast<int>(pClient->buf[1])](*this, pClient->userData);
#else
	moveFunctionArr[static_cast<int>(pClient->buf[1])](*this, pClient->userData);
#endif
}

void WorldManager::SendMoveCharacter(SocketInfo* pClient)
{
#ifdef _DEV_MODE_
	std::cout << "보낼 방향은" << int(pClient->userData->GetPosition().x) << " " << int(pClient->userData->GetPosition().y) << "\n";
#endif

	pClient->buf[1] = GLOBAL_UTIL::BIT_CONVERTER::MakeByteFromLeftAndRightByte(pClient->userData->GetPosition().x, pClient->userData->GetPosition().y);
}