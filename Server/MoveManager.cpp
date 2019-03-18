#include "pch.h"

#include "UserData.h"
#include "SocketInfo.h"

#include "MoveManager.h"

MoveManager::MoveManager() noexcept
{
#ifdef _NOT_IF_MOVE_
	MoveManager::moveFunctions[static_cast<int>(DIRECTION::LEFT)][0] = &MoveManager::MoveFail;
	MoveManager::moveFunctions[static_cast<int>(DIRECTION::LEFT)][1] = &MoveManager::MoveLeft;
	MoveManager::moveFunctions[static_cast<int>(DIRECTION::UP)][0] = &MoveManager::MoveFail;
	MoveManager::moveFunctions[static_cast<int>(DIRECTION::UP)][1] = &MoveManager::MoveUp;
	MoveManager::moveFunctions[static_cast<int>(DIRECTION::RIGHT)][0] = &MoveManager::MoveFail;
	MoveManager::moveFunctions[static_cast<int>(DIRECTION::RIGHT)][1] = &MoveManager::MoveRight;
	MoveManager::moveFunctions[static_cast<int>(DIRECTION::DOWN)][0] = &MoveManager::MoveFail;
	MoveManager::moveFunctions[static_cast<int>(DIRECTION::DOWN)][1] = &MoveManager::MoveDown;

#else
	MoveManager::moveFunctions[static_cast<int>(DIRECTION::LEFT)] = &MoveManager::MoveLeft;
	MoveManager::moveFunctions[static_cast<int>(DIRECTION::UP)] = &MoveManager::MoveUp;
	MoveManager::moveFunctions[static_cast<int>(DIRECTION::RIGHT)] = &MoveManager::MoveRight;
	MoveManager::moveFunctions[static_cast<int>(DIRECTION::DOWN] = &MoveManager::MoveDown;
#endif
}

void MoveManager::MoveCharacter(SocketInfo* pClient)
{
#ifdef _DEBUG_MODE_
	std::cout << "움직이는 방향은 : " << int(static_cast<int>(pClient->buf[1])) << "\n";
#endif

#ifdef _NOT_IF_MOVE_
	moveFunctions[static_cast<int>(pClient->buf[1])][](*this, pClient->userData);
#else
	moveFunctions[static_cast<int>(pClient->buf[1])](*this, pClient->userData);
#endif
}

void MoveManager::SendMoveCharacter(SocketInfo* pClient)
{
#ifdef _DEBUG_MODE_
	std::cout << "보낼 방향은" << int(pClient->userData->GetPosition().x) << " " << int(pClient->userData->GetPosition().y) << "\n";
#endif
	pClient->buf[1] = GLOBAL_UTIL::BIT_CONVERTER::MakeByteFromLeftAndRightByte(pClient->userData->GetPosition().x, pClient->userData->GetPosition().y);
}