#include "pch.h"
#include "Define.h"
#include "ServerDefine.h"

#include "UserData.h"
#include "MemoryUnit.h"

#include "SendMemoryPool.h"

#include "ClientContUnit.h"

#include "MoveManager.h"

MoveManager::MoveManager() noexcept
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

/*
	MoveCharacter()
		- pClient가 입력한 방향에 따라 처리해줍니다.

	!0. 해당 버퍼값이 Direction의 사이즈 보다 큰 값일 경우, Overflow가 발생합니다. 
*/
void MoveManager::MoveCharacter(SocketInfo* pClient)
{
#ifdef _DEV_MODE_
	std::cout << "움직이는 방향은 : " << int(static_cast<int>(pClient->loadedBuf[2])) << "\n";
#endif

#if _USE_STD_FUNCTION_
	whatIsYourDirection[static_cast<int>(pClient->loadedBuf[2])](*this, pClient->userData);
#else
	moveFunctionArr[static_cast<int>(pClient->buf[1])](*this, pClient->userData);
#endif
}

void MoveManager::SendMoveCharacter(SocketInfo* pMovedClient, SceneContUnit* inClientCont)
{
	//const BYTE clientPositionByte
	//	= BIT_CONVERTER::MakeByteFromLeftAndRightByte(pMovedClient->userData->GetPosition().x, pMovedClient->userData->GetPosition().y);

	PACKET_DATA::SC::Position packet(
		pMovedClient->clientKey,
		pMovedClient->userData->GetPosition().x, 
		pMovedClient->userData->GetPosition().y
	);

#ifdef _DEV_MODE_
	std::cout << "보낼 방향은" << int(pMovedClient->userData->GetPosition().x) << " " << int(pMovedClient->userData->GetPosition().y) << "\n";
#endif

	for (std::pair<bool, SocketInfo*>& pRecvedClient : inClientCont->clientCont)
	{
		if (pRecvedClient.first)
		{
			NETWORK_UTIL::SendPacket(pRecvedClient.second, reinterpret_cast<char*>(&packet));
		}
	}
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
	moveFunctionArr[DIRECTION::RIGHT][!(static_cast<bool>(inUserData->GetPosition().x / GLOBAL_DEFINE::MAX_WIDTH))](*this, inUserData);
};

void MoveManager::DownMoveTest(UserData* inUserData)
{
	moveFunctionArr[DIRECTION::DOWN][!(static_cast<bool>(inUserData->GetPosition().y / GLOBAL_DEFINE::MAX_HEIGHT))](*this, inUserData);
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