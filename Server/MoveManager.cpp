#include "pch.h"

#include "UserData.h"
#include "SocketInfo.h"

#include "MoveManager.h"

MoveManager::MoveManager() noexcept
{
	MoveManager::moveFunctions[0] = &MoveManager::MoveLeft;
	MoveManager::moveFunctions[1] = &MoveManager::MoveUp;
	MoveManager::moveFunctions[2] = &MoveManager::MoveRight;
	MoveManager::moveFunctions[3] = &MoveManager::MoveDown;
}

void MoveManager::MoveCharacter(SocketInfo* pClient)
{
	std::cout << "움직이는 방향은 : " << int(static_cast<int>(pClient->buf[1])) << "\n";
	moveFunctions[static_cast<int>(pClient->buf[1])](*this, pClient->userData);
}

void MoveManager::SendMoveCharacter(SocketInfo* pClient)
{
	std::cout << "보낼 방향은" << int(pClient->userData->GetPosition().x) << " " << int(pClient->userData->GetPosition().y) << "\n";
	pClient->buf[1] = GLOBAL_UTIL::BIT_CONVERTER::MakeByteFromLeftAndRightByte(pClient->userData->GetPosition().x, pClient->userData->GetPosition().y);
}