#pragma once

/*
	Define.h
		- 해당 헤더 파일은, 서버와 클라이언트가 공통으로 사용합니다. 
		(클라이언트에서 서버 프로젝트 코드 참조)

	#0. 해당 헤더는, Enum class, enum, Constexpr 상수 만을 포함해야합니다.
	
	!0. enum, 전역 상수의 경우, 반드시 namespace안에 선언되어야 합니다.
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
			LEFT,
			UP,
			RIGHT,
			DOWN,
			ENUM_SIZE
		};
	}

	namespace SERVER_TO_CLIENT
	{
		enum
		{
			LOGIN_OK,
			PUT_PLAYER,
			REMOVE_PLAYER,
			MOVE,
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
		struct Up
		{
			char size;
			char type;
		};

		struct Down
		{
			char size;
			char type;
		};

		struct Left
		{
			char size;
			char type;
		};

		struct Right
		{
			char size;
			char type;
		};
	}

	namespace SERVER_TO_CLIENT
	{
		struct LoginOk
		{
			char size;
			char type;
			char id;
		};

		struct PutPlayer
		{
			char size;
			char type;
			char id;
			char x;
			char y;
		};

		struct RemovePlayer
		{
			char size;
			char type;
			char id;
		};

		struct Position
		{
			char size;
			char type;
			char id;
			char x;
			char y;
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

namespace CHARACTER_CONVERTER {
	void SetLocaleToKorea();
}

namespace GLOBAL_DEFINE
{
	constexpr USHORT SERVER_PORT = 9000;
	constexpr BYTE MAX_HEIGHT = 7;
	constexpr BYTE MAX_WIDTH = 7;
	constexpr BYTE MAX_CLIENT = 10;

	// 아래 상수들은 추후 해당 헤더에서 추방, SERVER_DEFINE으로 위치 변경 필요합니다.
	constexpr USHORT MAX_SIZE_OF_RECV = 100;		//Recv 한번에 받을 수 있는 최대 사이즈
	constexpr USHORT MAX_SIZE_OF_RECV_PACKET = sizeof(PACKET_DATA::CLIENT_TO_SERVER::Down);	// (2) Recv 시, 처리해야하는 패킷 중 가장 큰 사이즈
	constexpr USHORT MAX_SIZE_OF_SEND = sizeof(PACKET_DATA::SERVER_TO_CLIENT::Position);	// (5) Send 시, 처리해야하는 패킷 중 가장 큰 사이즈
	constexpr USHORT MAX_NUMBER_OF_SEND_POOL = 100;
}