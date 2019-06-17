#pragma once
/*
	kpu-warp-winapi-framework		Copyright ⓒ https://github.com/KPU-WARP

	#0. 해당 프로젝트는 한국산업기술대 게임공학과 학술 소모임 WARP의 오픈소스 WinAPI Framework입니다.
	#1. 관련 세부 정보는 깃헙 저장소 리드미에서 확인하실 수 있습니다. 
		- https://github.com/KPU-WARP/winapi-framework
	
	#2. 기타 문의는, KoreaGameMaker@gmail.com으로 부탁드립니다. 감사합니다 :)
*/

/*
	Define.h
		- 해당 헤더 파일은, 서버와 클라이언트가 공통으로 사용합니다.
		(클라이언트에서 서버 프로젝트 코드 참조)

	#0. 해당 헤더는, 서버와 클라이언트

	!0. enum, 전역 상수, 전역 함수의 경우, 반드시 namespace안에 선언되어야 합니다.
		- 절대로 네임스페이스 없이, 변수명만으로 사용되는 경우를 방지합니다.
	!1. enum class, enum의 경우 마지막에 ENUM_SIZE를 포함해야합니다.
*/

namespace GLOBAL_DEFINE
{
	constexpr USHORT MAIN_SERVER_PORT = 9000;
	constexpr USHORT CHAT_SERVER_PORT = 9001;
	constexpr USHORT MANAGER_SERVER_PORT = 9002;
	constexpr USHORT QUERY_SERVER_PORT = 9003;

	constexpr USHORT MAX_HEIGHT = 300;
	constexpr USHORT MAX_WIDTH = 300;

	constexpr BYTE ID_MAX_LEN = 10;
	constexpr BYTE ID_MAX_SIZE = ID_MAX_LEN * 2;

	constexpr BYTE CHAT_MAX_LEN = 50;
}

namespace STAT_CHANGE
{
	enum
	{
		HP,
		MP,
		LEVEL,
		EXP,
		RED_P,
		BLUE_P,
		MONEY,
		MOVE_OK,
		ATTACK_OK,
		SKILL_1_OK,
		SKILL_2_OK
	};
}

namespace NETWORK_TYPE
{
	enum /*class NETWORK_TYPE : BYTE */
	{
		CLIENT_RECV_FROM_MAIN,/* = 0*/
		CLIENT_RECV_FROM_CHAT,
		CLIENT_SEND_TO_MAIN,
		CLIENT_SEND_TO_CHAT,

		MAIN_RECV_FROM_CLIENT,
		MAIN_SEND_TO_CLIENT,

		CHAT_RECV_FROM_CLIENT,
		CHAT_SEND_TO_CLIENT,

		COMMAND_SEND_TO_CHAT,
		COMMAND_SEND_TO_MAIN,

		ENUM_SIZE
	};
}

namespace PACKET_TYPE
{
	namespace CLIENT_TO_MAIN
	{
		enum
		{
			MOVE, 	//LEFT, //UP, //RIGHT, //DOWN,
			LOGIN,
			SIGN_UP,
			ATTACK,
			USE_ITEM,
			CHAT,
			ENUM_SIZE
		};
	}

	namespace CLIENT_TO_CHAT
	{
		enum
		{
			CHAT,	// CS::CHAT와 SC::CHAT는 동일해야합니다.
			CONNECT,	// 채팅 서버에 해당 클라이언트를 등록합니다.
			CHANGE,		// 해당 클라이언트의 존이 변경되었습니다.
			ENUM_SIZE
		};
	}

	namespace MAIN_TO_CLIENT
	{
		enum
		{
			POSITION,
			LOGIN_OK,
			LOGIN_FAIL,
			PUT_PLAYER,
			REMOVE_PLAYER,
			CHAT,
			STAT_CHANGE,
			ENUM_SIZE
		};
	}

	namespace MAIN_TO_QUERY
	{
		enum
		{
			DEMAND_LOGIN = 0,
			DEMAND_SIGNUP = 1,
			SAVE_LOCATION = 2,
			SAVE_USERINFO
		};
	}

	namespace QUERY_TO_MAIN
	{
		enum
		{
			LOGIN_TRUE,
			LOGIN_FALSE,
			LOGIN_ALREADY,
			LOGIN_NEW
		};
	}

	namespace CHAT_TO_CLIENT
	{
		enum
		{
			CHAT,	// CS::CHAT와 SC::CHAT는 동일해야합니다.
			URGENT_NOTICE,		// ChatServer에서 Client로 긴급 공지를 보낼때.
			ENUM_SIZE
		};
	}

	namespace COMMAND_TO_CHAT
	{
		enum
		{
			URGENT_NOTICE,	// CommandServer에서 ChatServer로 긴급공지 요청을 보냄.
			ENUM_SIZE
		};
	}
}

namespace PACKET_DATA
{
	using _PacketSizeType = const char;
	using _PacketTypeType = const char;	//? 이름이 뭐 이따구가
	using _KeyType = unsigned int;	//? 이름이 뭐 이따구가
	using _LevelType = unsigned int;
	using _PosType = unsigned short;
	using _CharType = WCHAR;
	using _ExpType = unsigned int;
	using _JobType = unsigned int;	//일단;
	using _HpType = unsigned int;
	using _MpType = unsigned int;
	using _MoneyType = unsigned int;
	using _RedCountType = unsigned int;
	using _BlueCountType = unsigned int;
	using _TreeCountType = unsigned int;

#pragma pack(push, 1)

	namespace CLIENT_TO_MAIN
	{
		struct Move {
			_PacketSizeType size;
			_PacketTypeType type;
			char direction;

			Move(const char inDirection) noexcept;
		};

		struct Login {
			_PacketSizeType size;
			_PacketTypeType type;
			_CharType id[GLOBAL_DEFINE::ID_MAX_LEN];

			Login(const _CharType* const pInNickname) noexcept;
		};

		struct SignUp {
			_PacketSizeType size;
			_PacketTypeType type;
			_CharType id[GLOBAL_DEFINE::ID_MAX_LEN];
			_JobType job;

			SignUp(const _CharType* const pInNickname, const _JobType) noexcept;
		};

		struct Attack {
			_PacketSizeType size;
			_PacketTypeType type;
			unsigned char attackType; //0이면 기본공격, 1이면 스킬1, 2이면 스킬2

			Attack(const unsigned char) noexcept;
		};

		struct Item {
			_PacketSizeType size;
			_PacketTypeType type;
			unsigned char useItemType;	//0이면 레드포션, 1이면 블루포션

			Item(const unsigned char) noexcept;
		};

		struct Chat {
			_PacketSizeType size;
			_PacketTypeType type;
			_CharType message[GLOBAL_DEFINE::CHAT_MAX_LEN];

			Chat(_CharType*);
		};
	}

	namespace CLIENT_TO_CHAT
	{
		struct Chat {
#ifdef NOT_USE
			char size;	// Fixed - 1	0
			const char type;	// Fixed - 1	1
			char nickNameLength;	// 1	2
			char messageLength;	// 1	2	// 패딩비트역활좀할쯤 여기1바이트넣자고냥
			std::wstring nickName;	// 1	
			std::wstring message;

			//message[0] = Length;				//Fixed
			//message[1] = type;					//Fixed
			//message[2] = nickNameLength;
			//message[3] = messageLength;

			//message[4] ~message[4 + nickNameLength * 2] = Nickname;
			//message[5 + nickNameLength * 2 + 1] ~message[Length] = ChatMessage;

			Chat(const char* pBufferStart);
#endif
			char message[80];
			Chat(const std::wstring& inNickName, const std::wstring& inMessage);	// 레퍼런스가 아닌, Copy합니다.
		};
	}

	namespace MAIN_TO_CLIENT
	{
		struct LoginOk
		{
			_PacketSizeType size;
			_PacketTypeType type;
			_KeyType key;
			//_CharType nickname[GLOBAL_DEFINE::ID_MAX_LEN];
			_PosType x;
			_PosType y;
			_LevelType level;
			_ExpType exp;
			_JobType job;
			_HpType hp;
			_MpType mp;
			_MoneyType money;
			_RedCountType redCount;
			_BlueCountType blueCount;
			_TreeCountType treeCount;

			LoginOk(const _KeyType, /*const _CharType* inNewNickname*/ const _PosType x, const _PosType y,
				_LevelType inlevel, _ExpType inExp, _JobType inJob, _HpType inHp, _MpType inMp,
				_MoneyType inMoney, _RedCountType inRedCount, _BlueCountType inBlueCount, _TreeCountType inTreeCount
			) noexcept;
		};

		struct LoginFail
		{
			_PacketSizeType size;
			_PacketTypeType type;
			const char failReason;	// 0이면 없는 계정, 1이면 이미 로그인한 계정, 2이면 만들려고 했는데 이미 있는 계정

			LoginFail(const char inFailReason) noexcept;
		};

		struct PutPlayer
		{
			_PacketSizeType size;
			_PacketTypeType type;
			_KeyType key;
			_PosType x;
			_PosType y;
			_JobType job;

			PutPlayer(const _KeyType inMovedClientKey, const _PosType inX, const _PosType inY, const _JobType inJob) noexcept;
		};

		struct RemovePlayer
		{
			_PacketSizeType size;
			_PacketTypeType type;
			_KeyType key;

			RemovePlayer(const _KeyType inRemovedClientKey) noexcept;
		};

		struct Position
		{
			_PacketSizeType size;
			_PacketTypeType type;
			_KeyType key;
			_PosType x;
			_PosType y;

			Position(const _KeyType inMovedClientKey, const _PosType inX, const _PosType inY) noexcept;
		};

		struct Chat
		{
			_PacketSizeType size;
			_PacketTypeType type;
			_KeyType key;
			_CharType message[GLOBAL_DEFINE::CHAT_MAX_LEN];

			Chat(const _KeyType inSenderKey, const _CharType* const pInMessage) noexcept;
		};

		struct StatChange
		{
			_PacketSizeType size;
			_PacketTypeType type;
			char changedStatType;
			// 0 체력, 1 마나, 2 레벨, 3. 경험치, 4. 빨물, 
			//5.파물, 6.돈, 7. 이동가능여부, 8. 공격가능여부, 9. 스킬1가능여부, 10. 스킬2가능여부
			int newValue;

			StatChange(char /* STAT_CHANGE */, int) noexcept;
		};
	}

	namespace MAIN_TO_QUERY
	{
		struct DemandLogin
		{
			_PacketSizeType size;
			_PacketTypeType type;
			_KeyType key;	// 나중에 반납할 때, 이 키를 알려줘야함.
			_CharType id[10];
			int		pw;

			DemandLogin(const _KeyType, const char* inId, const int);
		};

		struct DemandSignUp
		{
			_PacketSizeType size;
			_PacketTypeType type;
			_KeyType key;	// 나중에 반납할 때, 이 키를 알려줘야함.
			_CharType id[10];
			_JobType job;

			DemandSignUp(const _KeyType, const char* inId, const _JobType);
		};

		struct SavePosition
		{
			_PacketSizeType size;
			_PacketTypeType type;
			// 여기는 단방향성이라 Key가 필요없음
			_CharType id[10];
			_PosType xPos;
			_PosType yPos;
			SavePosition(const _CharType * const, const _PosType, const _PosType);
		};

		struct SaveUserInfo
		{
			_PacketSizeType size;
			_PacketTypeType type;
			// 여기는 단방향성이라 Key가 필요없음
			int isOut;	// 0 로그아웃용, 1 백업용
			_CharType id[10];
			_PosType xPos;
			_PosType yPos;
			_LevelType level;
			_ExpType exp;
			_JobType job;
			_HpType hp;
			_MpType mp;
			_MoneyType money;
			_RedCountType redCount;
			_BlueCountType blueCount;
			_TreeCountType treeCount;

			SaveUserInfo(const int inIsOut, const _CharType* const inId, const _PosType inXPos, const _PosType inYPos,
				const _LevelType inLevel, const _ExpType exp, const _JobType job, const _HpType hp, const _MpType mp,
				const _MoneyType money, const _RedCountType redCount, const _BlueCountType blueCount, const _TreeCountType treeCount);
		};
	}

	namespace CHAT_TO_CLIENT
	{
		struct Chat
		{
			// 부하를 줄이기 위해, 채팅은 릴레이 방식으로 활용
			char message[80];
			Chat(char*);
		};
	}

	namespace QUERY_TO_MAIN
	{
		struct LoginTrue
		{
			_PacketSizeType size;
			_PacketTypeType type;
			_KeyType key;
			//_CharType nickname[10];
			_PosType xPos;
			_PosType yPos;
			_LevelType level;
			_ExpType exp;
			_JobType job;
			_HpType hp;
			_MpType mp;
			_MoneyType money;
			_RedCountType redCount;
			_BlueCountType blueCount;
			_TreeCountType treeCount;

			LoginTrue(const _KeyType/*, const _KeyType,  const _CharType *, const _PosType, const _PosType*/) noexcept;
		};

		struct LoginFail
		{
			_PacketSizeType size;
			_PacketTypeType type;
			_KeyType key;
			unsigned char failReason;

			LoginFail(const _KeyType, const unsigned char) noexcept;
		};

		struct LoginAlready
		{
			_PacketSizeType size;
			_PacketTypeType type;
			_KeyType key;
			_KeyType oldKey;

			LoginAlready(const _KeyType, const _KeyType) noexcept;
		};

		struct LoginNew
		{
			_PacketSizeType size;
			_PacketTypeType type;
			_KeyType key;
			_JobType job;

			LoginNew(const _KeyType, const _JobType) noexcept;
		};
	}

#pragma pack(pop)
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

namespace JOB_TYPE
{
	enum /* : int */
	{
		KNIGHT = 1,
		ARCHER = 2,
		WITCH = 3,
		SLIME = 4,
		GOLEM = 5,
		DRAGON = 6
	};
}

namespace UNICODE_UTIL
{
	void SetLocaleToKorean();

	_NODISCARD std::string WStringToString(const std::wstring& InWstring);
	_NODISCARD std::wstring StringToWString(const std::string& InString);
}

namespace BIT_CONVERTER
{
	constexpr BYTE SEND_BYTE = (1 << 7);
	constexpr unsigned int NOT_PLAYER_INT = (1 << 31);	// 0일때는 플레이어 바이트, 1일때는 2차검사 필요.
	constexpr unsigned int NPC_INT = (1 << 30);	// 0일 때는 몬스터, 1일 때는 NPC
	constexpr unsigned int REAL_INT = 0x3fffffff;	// 0, 1 비트 마스크를 씌울 때 사용하는 변수.

	enum class OBJECT_TYPE : BYTE
	{
		PLAYER,
		MONSTER,
		NPC
	};

	std::pair<OBJECT_TYPE, unsigned int> WhatIsYourTypeAndRealKey(unsigned int) noexcept;
	unsigned int MakeMonsterKey(unsigned int) noexcept;

	/*_NODISCARD*/ BYTE MakeSendPacket(const BYTE inPacketType) noexcept;
	/*_NODISCARD*/ bool GetRecvOrSend(const char inChar) noexcept;

	/*_NODISCARD*/ BYTE MakeByteFromLeftAndRightByte(const BYTE inLeftByte, const BYTE inRightByte) noexcept;
	/*_NODISCARD*/ BYTE GetLeft4Bit(const BYTE inByte) noexcept;
	/*_NODISCARD*/ BYTE GetRight4Bit(const BYTE inByte) noexcept;
}