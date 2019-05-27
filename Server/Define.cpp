#include "pch.h"
#include "Define.h"

namespace PACKET_DATA
{
	namespace CLIENT_TO_MAIN
	{
		Move::Move(char inDirection) noexcept :
			size(sizeof(Move)), type(PACKET_TYPE::CLIENT_TO_MAIN::MOVE),
			direction(inDirection)
		{}
	}

	namespace CLIENT_TO_CHAT
	{
#ifdef NOT_USE
		Chat::Chat(const char* inRecvBuffer) :
			size(inRecvBuffer[0]), type(PACKET_TYPE::CS::CHAT_SERVER_CHAT),
			nickNameLength(inRecvBuffer[2]),
			nickName(),
			message()
		{
			assert(false, L"이 생성자 Chat는 이제 안쓰여요! 다만 버퍼 카운터를 위해 남겨둡시다.");
			std::string stringNickname(inRecvBuffer[3], inRecvBuffer[3 + nickNameLength]);
			nickName = std::move(UNICODE_UTIL::StringToWString(stringNickname));

			std::string stringMessage(inRecvBuffer[3 + nickNameLength + 1],
				inRecvBuffer[size - 1]);
			message = std::move(UNICODE_UTIL::StringToWString(stringMessage));
		}
#endif
		Chat::Chat(const std::wstring& inNickName, const std::wstring& inMessage)
		{
			const BYTE nickNameLength = inNickName.size() * 2; // BYTE
			const BYTE messageLength = inMessage.size() * 2;	//BYTE

			message[0] = nickNameLength + messageLength + 4;
			message[1] = PACKET_TYPE::CLIENT_TO_CHAT::CHAT;
			message[2] = nickNameLength;
			message[3] = messageLength;

			memcpy(message + 4, UNICODE_UTIL::WStringToString(/*const_cast<std::wstring&>*/(inNickName)).c_str(), nickNameLength);
			memcpy(message + (4 + nickNameLength), UNICODE_UTIL::WStringToString(inMessage).c_str(), messageLength);
		}
	}
	
	namespace MAIN_TO_CLIENT
	{
		LoginOk::LoginOk(const UINT inNewId) noexcept :
			size(sizeof(LoginOk)), type(PACKET_TYPE::MAIN_TO_CLIENT::LOGIN_OK),
			id(inNewId)
		{}

		PutPlayer::PutPlayer(const UINT inPutClientId, const USHORT inX, const USHORT inY) noexcept :
			size(sizeof(PutPlayer)), type(PACKET_TYPE::MAIN_TO_CLIENT::PUT_PLAYER),
			id(inPutClientId),
			x(inX),
			y(inY)
		{}

		RemovePlayer::RemovePlayer(const UINT inRemovedClientID) noexcept :
			size(sizeof(RemovePlayer)), type(PACKET_TYPE::MAIN_TO_CLIENT::REMOVE_PLAYER),
			id(inRemovedClientID)
		{}

		Position::Position(const UINT inMovedClientId, const USHORT inX, const USHORT inY) noexcept :
			size(sizeof(Position)), type(PACKET_TYPE::MAIN_TO_CLIENT::POSITION),
			id(inMovedClientId),
			x(inX),
			y(inY)
		{}
	}

	namespace MAIN_TO_QUERY
	{
		DemandLogin::DemandLogin(const int inKey, WCHAR* inId, int inPw)
			: size(sizeof(DemandLogin)), type(PACKET_TYPE::MAIN_TO_QUERY::DEMAND_LOGIN),
			key(inKey), id(), pw(inPw)
		{
			for (int i = 0; i < 10; ++i)
			{
				id[i] = inId[i];
			}
		}

		SavePosition::SavePosition(WCHAR* inId, const int inXPos, const int inYPos)
			: size(sizeof(DemandLogin)), type(PACKET_TYPE::MAIN_TO_QUERY::DEMAND_LOGIN),
			id(), xPos(inXPos), yPos(inYPos)
		{
			for (int i = 0; i < 10; ++i)
			{
				id[i] = inId[i];
			}
		}
	}

	namespace CHAT_TO_CLIENT
	{
		Chat::Chat(char* inPtr)
		{
			memcpy(message, inPtr, static_cast<BYTE>(inPtr[0]));
		}
	}

	namespace QUERY_TO_MAIN
	{
		LoginTrue::LoginTrue(const int inKey, const int inX, const int inY) noexcept : 
			size(sizeof(LoginTrue)), type(PACKET_TYPE::QUERY_TO_MAIN::LOGIN_TRUE),
			key(inKey),
			xPos(inX), yPos(inY)
		{}

		LoginFail::LoginFail(const int inKey, const unsigned char inFailReason) noexcept :
			size(sizeof(LoginTrue)), type(PACKET_TYPE::QUERY_TO_MAIN::LOGIN_FALSE),
			key(inKey),
			failReason(inFailReason)
		{}
	}
}

namespace UNICODE_UTIL
{
	void SetLocaleToKorean()
	{
		_wsetlocale(LC_ALL, L"Korean");

		// ? 계속 오류 내뱉어서 일단 꺼놓음.
		//26444 왜 때문에, 굳이 필요 없이, L-Value를 만들어야하는가;
		/*auto oldLocale = std::wcout.imbue(std::locale("koeran")); */
	}

	_NODISCARD std::string WStringToString(const std::wstring& InWString)
	{
		const int sizeBuffer = WideCharToMultiByte(CP_ACP, 0, &InWString[0], -1, NULL, 0, NULL, NULL);

		std::string retString(sizeBuffer, 0);

		WideCharToMultiByte(CP_ACP, 0, &InWString[0], -1, &retString[0], sizeBuffer, NULL, NULL);

		// FixError ==
		retString.pop_back(); //(retString.end(), retString.end());
		//retString.insert(retString.end(), '\0');

		return retString;
	}

	_NODISCARD std::wstring StringToWString(const std::string& InString)
	{
		const int sizeBuffer = MultiByteToWideChar(CP_ACP, 0, &InString[0], -1, NULL, 0);

		std::wstring retString(sizeBuffer, 0);

		MultiByteToWideChar(CP_ACP, 0, &InString[0], -1, &retString[0], sizeBuffer);

		return retString;
	}
}

namespace BIT_CONVERTER
{
	std::pair<OBJECT_TYPE, unsigned int> WhatIsYourTypeAndRealKey(unsigned int inKey) noexcept
	{
		std::pair<OBJECT_TYPE, unsigned int> retPair;

		//if ((inKey & NOT_PLAYER_INT) == inKey) retPair.first = OBJECT_TYPE::PLAYER;
		//else if ((inKey & NPC_INT) != inKey) retPair.first = OBJECT_TYPE::MONSTER;
		//else retPair.first = OBJECT_TYPE::NPC;
		if (inKey < NOT_PLAYER_INT) retPair.first = OBJECT_TYPE::PLAYER;
		else  if ((inKey < NOT_PLAYER_INT + NPC_INT)) retPair.first = OBJECT_TYPE::MONSTER;
		else  retPair.first = OBJECT_TYPE::NPC;

		retPair.second = inKey & REAL_INT;

		return retPair;
	}

	unsigned int MakeMonsterKey(unsigned int inOnlyIndex) noexcept
	{
		return (inOnlyIndex | NOT_PLAYER_INT);
	}

	/*
		MakeByteFromLeftAndRightByte()
			- 인자로 들어온 패킷 타입(바이트)의 최상위 비트를 1로 바꾼후 반환합니다.

		!0. 패킷 타입 개수가, ox7f보다 클 경우, 해당 함수 및 현재 서버 로직은 오류가 발생합니다.
	*/
	BYTE MakeSendPacket(const BYTE inPacketType) noexcept { return inPacketType | SEND_BYTE; }

	/*
		MakeByteFromLeftAndRightByte()
			- 인자로 들어온 패킷 타입(바이트)의 최상위 비트를 검사합니다.

		#0. 최상위 비트가 1일 경우, true를, 0일 경우 false를 반환합니다. (형변환을 통한 Array Overflow 방지 )

		!0. 패킷 타입 개수가, ox7f보다 클 경우, 해당 함수 및 현재 서버 로직은 오류가 발생합니다.
	*/
	bool GetRecvOrSend(const char inChar) noexcept { return (inChar >> 7) & (0x01); }

	/*
		MakeByteFromLeftAndRightByte
			- 0x10보다 작은 바이트 두개를 인자로 받아(Left, Right) 상위 4개비트는 Left, 하위 4개 비트는 Right를 담아반환합니다.

		!0. 인자로 들어오는 두 바이트 모두, 0x10보다 작은 값이 들어와야합니다.
			- 인자로 들어오는 Left 바이트가 0x0f보다 큰 값이 들어올 경우, 오버플로우되어 비정상적인 값이 반횐될 수 있음.
			- 인자로 들어오는 Right 바이트가 0x0f보다 큰 값이 들어올 경우, LeftByte의 | 연산에서 비정상적인 값을 반환할 수 있음.
	*/
	BYTE MakeByteFromLeftAndRightByte(const BYTE inLeftByte, const BYTE inRightByte) noexcept
	{
		return (inLeftByte << 4) | (inRightByte);
	}

	/*
		GetLeft4Bit
			- HIWORD(?)와 유사하게 동작합니다. 하나의 바이트를 받아서 상위(좌측) 4개의 비트를 바이트로 변환해서 반환합니다.
	*/
	BYTE GetLeft4Bit(const BYTE inByte) noexcept
	{
		return (inByte >> 4) & (0x0f);
	}

	/*
		GetRight4Bit
			- LOWORD(?)와 유사하게 동작합니다. 하나의 바이트를 받아서 하위(우측) 4개의 비트를 바이트로 변환해서 반환합니다.
	*/
	BYTE GetRight4Bit(const BYTE inByte) noexcept
	{
		return (inByte) & (0x0f);
	}
}