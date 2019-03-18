#include "pch.h"

#include "UserData.h"
#include "SocketInfo.h"

#include "MoveManager.h"

MoveManager::MoveManager() noexcept
{
#ifdef _NOT_IF_MOVE_
#ifdef _USE_LAMBDA_
	whatIsYourDirection[DIRECTION::LEFT] = [](MoveManager& inMoveManager, UserData* inUserData)->void 
	{
		inMoveManager.moveFunctionArr[DIRECTION::LEFT][static_cast<bool>(inUserData->GetPosition().x)](inMoveManager, inUserData);
	};
	whatIsYourDirection[DIRECTION::UP] = &MoveManager::UpMoveTest;
	whatIsYourDirection[DIRECTION::RIGHT] = &MoveManager::RightMoveTest;
	whatIsYourDirection[DIRECTION::DOWN] = &MoveManager::DownMoveTest;
	
	MoveManager::moveFunctionArr[DIRECTION::LEFT][0] = [](MoveManager&, UserData* inUserData)->void {};
	MoveManager::moveFunctionArr[DIRECTION::UP][0] = [](MoveManager&, UserData* inUserData)->void {};
	MoveManager::moveFunctionArr[DIRECTION::RIGHT][0] = [](MoveManager&, UserData* inUserData)->void {};
	MoveManager::moveFunctionArr[DIRECTION::DOWN][0] = [](MoveManager&, UserData* inUserData)->void {};

	MoveManager::moveFunctionArr[DIRECTION::LEFT][1] = [](MoveManager&, UserData* inUserData)->void 
	{
		inUserData->SetPosition(inUserData->GetPosition().x - 1, inUserData->GetPosition().y);
	};
	MoveManager::moveFunctionArr[DIRECTION::UP][1] = [](MoveManager&, UserData* inUserData)->void
	{
		inUserData->SetPosition(inUserData->GetPosition().x, inUserData->GetPosition().y - 1);
	};
	MoveManager::moveFunctionArr[DIRECTION::RIGHT][1] = [](MoveManager&, UserData* inUserData)->void
	{
		inUserData->SetPosition(inUserData->GetPosition().x + 1, inUserData->GetPosition().y);
	};
	MoveManager::moveFunctionArr[DIRECTION::DOWN][1] = [](MoveManager&, UserData* inUserData)->void
	{
		inUserData->SetPosition(inUserData->GetPosition().x, inUserData->GetPosition().y + 1);
	};
#else
	whatIsYourDirection[DIRECTION::LEFT] = &MoveManager::LeftMoveTest;
	whatIsYourDirection[DIRECTION::UP] = &MoveManager::UpMoveTest;
	whatIsYourDirection[DIRECTION::RIGHT] = &MoveManager::RightMoveTest;
	whatIsYourDirection[DIRECTION::DOWN] = &MoveManager::DownMoveTest;

	MoveManager::moveFunctionArr[DIRECTION::LEFT][0] = &MoveManager::MoveFail;//&MoveManager::MoveFail;
	MoveManager::moveFunctionArr[DIRECTION::LEFT][1] = &MoveManager::MoveLeft;
	MoveManager::moveFunctionArr[DIRECTION::UP][0] = &MoveManager::MoveFail;
	MoveManager::moveFunctionArr[DIRECTION::UP][1] = &MoveManager::MoveUp;
	MoveManager::moveFunctionArr[DIRECTION::RIGHT][0] = &MoveManager::MoveFail;
	MoveManager::moveFunctionArr[DIRECTION::RIGHT][1] = &MoveManager::MoveRight;
	MoveManager::moveFunctionArr[DIRECTION::DOWN][0] = &MoveManager::MoveFail;
	MoveManager::moveFunctionArr[DIRECTION::DOWN][1] = &MoveManager::MoveDown;
#endif

#else
	MoveManager::moveFunctions[static_cast<int>(DIRECTION::LEFT)] = &MoveManager::MoveLeft;
	MoveManager::moveFunctions[static_cast<int>(DIRECTION::UP)] = &MoveManager::MoveUp;
	MoveManager::moveFunctions[static_cast<int>(DIRECTION::RIGHT)] = &MoveManager::MoveRight;
	MoveManager::moveFunctions[static_cast<int>(DIRECTION::DOWN)] = &MoveManager::MoveDown;
#endif
}

void MoveManager::MoveCharacter(SocketInfo* pClient)
{
#ifdef _DEV_MODE_
	std::cout << "움직이는 방향은 : " << int(static_cast<int>(pClient->buf[1])) << "\n";
#endif

#ifdef _NOT_IF_MOVE_
	whatIsYourDirection[static_cast<int>(pClient->buf[1])](*this, pClient->userData);
#else
	moveFunctions[static_cast<int>(pClient->buf[1])](*this, pClient->userData);
#endif
}

void MoveManager::SendMoveCharacter(SocketInfo* pClient)
{
#ifdef _DEV_MODE_
	std::cout << "보낼 방향은" << int(pClient->userData->GetPosition().x) << " " << int(pClient->userData->GetPosition().y) << "\n";
#endif
	pClient->buf[1] = GLOBAL_UTIL::BIT_CONVERTER::MakeByteFromLeftAndRightByte(pClient->userData->GetPosition().x, pClient->userData->GetPosition().y);
}