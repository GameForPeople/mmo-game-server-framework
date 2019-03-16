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
	moveFunctions[static_cast<int>(pClient->buf[1])](*this, pClient->userData);
}

void MoveManager::SendMoveCharacter(SocketInfo* pClient)
{
	pClient->buf[1] = pClient->userData->GetPosition().x << 4 | pClient->userData->GetPosition().y;
}