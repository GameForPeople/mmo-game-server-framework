#pragma once

/*
	InHeaderDefine

	!0. �ش� �����, �����ϰ� �ٸ� ��� ���Ͽ� ���ԵǴ� ����Դϴ�.
*/

struct SocketInfo;

namespace std {
	template <class _Ty1, class _Ty2> struct pair;
}

using _ClientNode = std::pair<bool, SocketInfo*>;
//using _ClientCont = std::vector<_ClientNode>;

using _KeyType = int;
using _ClientKeyType = _KeyType;
using _MonsterKeyType = _KeyType;
using _NpcKeyType = _KeyType;

namespace GLOBAL_DEFINE
{
	//---------
	constexpr unsigned char MAX_CLIENT{ 10 };
	constexpr unsigned short MAX_SIZE_OF_RECV{ 100 };		//Recv �ѹ��� ���� �� �ִ� �ִ� ������
	constexpr unsigned short MAX_SIZE_OF_RECV_PACKET{ 80 };	//sizeof(PACKET_DATA::CLIENT_TO_SERVER::Chat);	// (2) Recv ��, ó���ؾ��ϴ� ��Ŷ �� ���� ū ������
	constexpr unsigned short MAX_SIZE_OF_SEND{ 5 }; // sizeof(PACKET_DATA::SERVER_TO_CLIENT::Position);	// (5) Send ��, ó���ؾ��ϴ� ��Ŷ �� ���� ū ������
	constexpr unsigned short MAX_NUMBER_OF_SEND_POOL{ 100 };
	//---
	constexpr unsigned char MAX_CHAT_MASSAGE_SIZE { 80 };
}

enum class OBJECT_TYPE : BYTE
{
	PLAYER,
	MONSTER,
	NPC
};