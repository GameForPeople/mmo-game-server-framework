#pragma once

/*
	InHeaderDefine

	!0. 해당 헤더는, 유일하게 다른 헤더 파일에 포함되는 헤더입니다.
*/

namespace std {
	template <class _Ty1, class _Ty2> struct pair;
}

//struct SocketInfo;
//using _ClientNode = std::pair<bool, SocketInfo*>;
//using _ClientCont = std::vector<_ClientNode>;

using _BufferType = char;

using _KeyType = unsigned int;
using _ClientKeyType = _KeyType;
using _MonsterKeyType = _KeyType;
using _NpcKeyType = _KeyType;

using _NicknameType = std::wstring;

using _PosType = unsigned short;

using _SectorIndexType = unsigned char;

enum class OBJECT_TYPE : unsigned char
{
	PLAYER,
	MONSTER,
	NPC
};

namespace GLOBAL_DEFINE
{
	//---------
	constexpr unsigned short MAX_SIZE_OF_RECV{ 100 };			//Recv 한번에 받을 수 있는 최대 사이즈
	constexpr unsigned short MAX_SIZE_OF_RECV_PACKET{ 80 };		//sizeof(PACKET_DATA::CLIENT_TO_SERVER::Chat);	// (2) Recv 시, 처리해야하는 패킷 중 가장 큰 사이즈
	constexpr unsigned short MAX_SIZE_OF_SEND{ 5 };				// sizeof(PACKET_DATA::SERVER_TO_CLIENT::Position);	// (5) Send 시, 처리해야하는 패킷 중 가장 큰 사이즈
	constexpr unsigned short MAX_NUMBER_OF_SEND_POOL{ 1000 };
	//---

	constexpr unsigned char MAX_CHAT_MASSAGE_SIZE { 80 };
}
