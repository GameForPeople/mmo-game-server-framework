#include "pch.h"
#include "ServerDefine.h"

#include "SocketInfo.h"
#include "UserData.h"

#include "MoveManager.h"

MoveManager::MoveManager()
{
#if _USE_STD_FUNCTION_
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
#else
	MoveManager::moveFunctionArr[DIRECTION::LEFT] = &MoveManager::MoveLeft;
	MoveManager::moveFunctionArr[DIRECTION::UP] = &MoveManager::MoveUp;
	MoveManager::moveFunctionArr[DIRECTION::RIGHT] = &MoveManager::MoveRight;
	MoveManager::moveFunctionArr[DIRECTION::DOWN] = &MoveManager::MoveDown;
#endif
}

void MoveManager::MoveCharacter(SocketInfo* pClient)
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

void MoveManager::SendMoveCharacter(SocketInfo* pClient)
{
#ifdef _DEV_MODE_
	std::cout << "보낼 방향은" << int(pClient->userData->GetPosition().x) << " " << int(pClient->userData->GetPosition().y) << "\n";
#endif
	pClient->buf[1] = GLOBAL_UTIL::BIT_CONVERTER::MakeByteFromLeftAndRightByte(pClient->userData->GetPosition().x, pClient->userData->GetPosition().y);
}

#if _USE_STD_FUNCTION_
void MoveManager::LeftMoveTest(UserData* inUserData)
{
	moveFunctionArr[DIRECTION::LEFT][static_cast<bool>(inUserData->GetPosition().x)](*this, inUserData);
};

void MoveManager::UpMoveTest(UserData* inUserData)
{
	moveFunctionArr[DIRECTION::UP][static_cast<bool>(inUserData->GetPosition().y)](*this, inUserData);
};

void MoveManager::RightMoveTest(UserData* inUserData)
{
	moveFunctionArr[DIRECTION::RIGHT][!(static_cast<bool>(inUserData->GetPosition().x / 7))](*this, inUserData);
};

void MoveManager::DownMoveTest(UserData* inUserData)
{
	moveFunctionArr[DIRECTION::DOWN][!(static_cast<bool>(inUserData->GetPosition().y / 7))](*this, inUserData);
};

void MoveManager::MoveFail(UserData* inUserData) noexcept
{}

void MoveManager::MoveLeft(UserData* inUserData) noexcept
{
	//Position2D newPosition = inUserData->GetPosition();
	//--newPosition.x;
	//inUserData->SetPosition(newPosition);
	inUserData->SetPosition(inUserData->GetPosition().x - 1, inUserData->GetPosition().y);
}

void MoveManager::MoveUp(UserData* inUserData) noexcept
{
	//Position2D newPosition = inUserData->GetPosition();
	//--newPosition.y;
	//inUserData->SetPosition(newPosition);
	inUserData->SetPosition(inUserData->GetPosition().x, inUserData->GetPosition().y - 1);
}

void MoveManager::MoveRight(UserData* inUserData) noexcept
{
	//Position2D newPosition = inUserData->GetPosition();
	//++newPosition.x;
	//inUserData->SetPosition(newPosition);
	inUserData->SetPosition(inUserData->GetPosition().x + 1, inUserData->GetPosition().y);
}

void MoveManager::MoveDown(UserData* inUserData) noexcept
{
	//Position2D newPosition = inUserData->GetPosition();
	//++newPosition.y;
	//inUserData->SetPosition(newPosition);
	inUserData->SetPosition(inUserData->GetPosition().x, inUserData->GetPosition().y + 1);
}
#endif