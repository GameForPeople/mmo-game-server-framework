#pragma once

namespace GLOBAL_DEFINE
{
	constexpr UINT8	MAIN_TIMER{ 1 };
	constexpr UINT8 MAIN_TIMER_FRAME{ 17 };

	constexpr UINT16 FRAME_WIDTH{ 1000 };
	constexpr UINT16 FRAME_HEIGHT{ 835 };

	constexpr UINT16 PLAY_FRAME_HALF_WIDTH{ 350 };
	constexpr UINT16 PLAY_FRAME_HALF_HEIGHT{ 350 };

	constexpr UINT8 MAX_LOADSTRING{ 100 };

	constexpr USHORT MAX_SIZE_OF_RECV{ 100 };		//Recv 한번에 받을 수 있는 최대 사이즈
	constexpr USHORT MAX_SIZE_OF_SEND{ 100 };		//sizeof(PACKET_DATA::CLIENT_TO_SERVER::Down);		//Recv 한번에 받을 수 있는 최대 사이즈

	constexpr UINT8 BLOCK_WIDTH_SIZE{ 40 };
	constexpr UINT8 BLOCK_HEIGHT_SIZE{ 40 };
}

namespace ERROR_HANDLING {
	// 해당 static Function Array의 초기화는 NetworkManager
	static std::function<void(void)> errorRecvOrSendArr[2];

	inline void NotError(void) {};
	void HandleRecvOrSendError(void);

	_NORETURN void ERROR_QUIT(const WCHAR* msg);
	/*_DEPRECATED*/ void ERROR_DISPLAY(const CHAR* msg);
}

namespace COLOR
{
	constexpr DWORD _RED = RGB(255, 0, 0);
	constexpr DWORD _GREEN = RGB(0, 255, 0);
	constexpr DWORD _BLUE = RGB(0, 0, 255);
	constexpr DWORD _WHITE = RGB(255, 255, 255);
	constexpr DWORD _BLACK = RGB(0, 0, 0);
}

enum class RENDER_MODEL_TYPE : BYTE
{
	PLAYER_KNIGHT,
	PLAYER_ARCHER,
	PLAYER_WITCH,

	OTHER_PLAYER_KNIGHT,
	OTHER_PLAYER_ARCHER,
	OTHER_PLAYER_WITCH,

	MONSTER_SLIME,
	MONSTER_GOLME,
	MONSTER_DRAGON,
	
	BACKGROUND_0,
	BACKGROUND_1,
	BACKGROUND_2,
	BACKGROUND_3,
	BACKGROUND_4,

	//NUMBER_0,
	//NUMBER_1,
	//NUMBER_2,
	//NUMBER_3,
	//NUMBER_4,
	//NUMBER_5,
	//NUMBER_6,
	//NUMBER_7,
	//NUMBER_8,
	//NUMBER_9,

	COVER_UI,
	BROADCAST_UI,

	ENUM_SIZE
};

// KeyBoard
enum class VK_KEY
{
	VK_0 = 48,
	VK_1 = 49,
	VK_2,
	VK_3,
	VK_4,
	VK_5,
	VK_6,
	VK_7,
	VK_8,
	VK_9,
	VK_A = 0x41,
	VK_B,
	VK_C,
	VK_D,
	VK_E,
	VK_F,
	VK_G,
	VK_H,
	VK_I,
	VK_J,
	VK_K,
	VK_L,
	VK_M,
	VK_N,
	VK_O,
	VK_P,
	VK_Q,
	VK_R,
	VK_S,
	VK_T,
	VK_U,
	VK_V,
	VK_W,
	VK_X,
	VK_Y,
	VK_Z
};