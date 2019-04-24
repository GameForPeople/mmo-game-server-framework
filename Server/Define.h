#pragma once

//--------------------- for DEV_MODE
//#define _DEV_MODE_
//---------------------


/*
	Define.h
		- 해당 헤더 파일은, 서버와 클라이언트가 공통으로 사용합니다.
		(클라이언트에서 서버 프로젝트 코드 참조)

	#0. 해당 헤더는, 서버와 클라이언트

	!0. enum, 전역 상수, 전역 함수의 경우, 반드시 namespace안에 선언되어야 합니다.
		- 절대로 네임스페이스 없이, 변수명만으로 사용되는 경우를 방지합니다.
	!1. enum class, enum의 경우 마지막에 ENUM_SIZE를 포함해야합니다.
*/


//--------------------- for DEV_MODE
//#define _DEV_MODE_
//---------------------

/*
	Define.h
		- 해당 헤더 파일은, 서버와 클라이언트가 공통으로 사용합니다.
		(클라이언트에서 서버 프로젝트 코드 참조)

	#0. 해당 헤더는, 서버와 클라이언트

	!0. enum, 전역 상수, 전역 함수의 경우, 반드시 namespace안에 선언되어야 합니다.
		- 절대로 네임스페이스 없이, 변수명만으로 사용되는 경우를 방지합니다.
	!1. enum class, enum의 경우 마지막에 ENUM_SIZE를 포함해야합니다.
*/

namespace NETWORK_TYPE
{
	enum /*class NETWORK_TYPE : BYTE */
	{
		RECV /* = 0*/,
		SEND,
		ENUM_SIZE
	};
}

namespace PACKET_TYPE
{
	namespace CLIENT_TO_SERVER
	{
		enum
		{
			MOVE, 	//LEFT, //UP, //RIGHT, //DOWN,
			CHAT,	// CS::CHAT와 SC::CHAT는 동일해야합니다.
			ENUM_SIZE
		};
	}

	namespace SERVER_TO_CLIENT
	{
		enum
		{
			POSITION,
			CHAT,	// CS::CHAT와 SC::CHAT는 동일해야합니다.
			LOGIN_OK,
			PUT_PLAYER,
			REMOVE_PLAYER,
			ENUM_SIZE
		};
	}

	namespace CS = CLIENT_TO_SERVER;
	namespace SC = SERVER_TO_CLIENT;
}

namespace PACKET_DATA
{
#pragma pack(push, 1)

	namespace CLIENT_TO_SERVER
	{
		struct Move {
			const char size;
			const char type;
			char direction;

			Move(char inDirection) noexcept;
		};
	}

	namespace SERVER_TO_CLIENT
	{
		struct LoginOk
		{
			const char size;
			const char type;
			char id;

			LoginOk(const char inNewId) noexcept;
		};

		struct PutPlayer
		{
			const char size;
			const char type;
			char id;
			char x;
			char y;

			PutPlayer(const char inMovedClientId, const char inX, const char inY) noexcept;
		};

		struct RemovePlayer
		{
			const char size;
			const char type;
			char id;

			RemovePlayer(const char inRemovedClientID) noexcept;
		};

		struct Position
		{
			const char size;
			const char type;
			char id;
			char x;
			char y;

			Position(const char inMovedClientId, const char inX, const char inY) noexcept;
		};

		struct Chat {	// 해당 구조체는 서버 코드에서 사용되지 않습니다.
			char size;	// Fixed - 1	0
			const char type;	// Fixed - 1	1
			char nickNameLength;	// 1	2
			std::wstring nickName;	// 1	
			std::wstring message;

			//message[0] = Length;				//Fixed
			//message[1] = type;					//Fixed
			//message[2] = nickNameLength;
			//message[3] ~message[3 + nickNameLength * 2] = Nickname;
			//message[3 + nickNameLength * 2 + 1] ~message[Length] = ChatMessage;

			Chat(const char* pBufferStart);
		};
	}

#pragma pack(pop)

	namespace CS = CLIENT_TO_SERVER;
	namespace SC = SERVER_TO_CLIENT;
}

namespace DIRECTION
{
	enum /* class DIRECTION : BYTE */
	{
		LEFT /*= 0*/,
		UP /*= 1*/,
		RIGHT /*= 2*/,
		DOWN /*= 3*/,
		ENUM_SIZE
	};
}

namespace UNICODE_UTIL
{
	void SetLocaleToKorean();

	_NODISCARD std::string WStringToString(std::wstring& InWstring);
	_NODISCARD std::wstring StringToWString(std::string& InString);
}

namespace GLOBAL_DEFINE
{
	constexpr USHORT SERVER_PORT = 9000;

	constexpr BYTE MAX_HEIGHT = 100;
	constexpr BYTE MAX_WIDTH = 100;
}