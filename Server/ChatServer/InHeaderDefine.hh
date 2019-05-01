#pragma once

/*
	InHeaderDefine

	!0. 해당 헤더는, 유일하게 다른 헤더 파일에 포함되는 헤더입니다.
*/

struct SocketInfo;

namespace std {
	template <class _Ty1, class _Ty2> struct pair;
}

using _ClientNode = std::pair<bool, SocketInfo*>;
using _ClientCont = std::vector<_ClientNode>;
using _ClientKeyType = int;

namespace GLOBAL_DEFINE
{
	//---------
	constexpr BYTE MAX_CLIENT{ 10 };
	constexpr USHORT MAX_SIZE_OF_RECV{ 100 };		//Recv 한번에 받을 수 있는 최대 사이즈
	constexpr USHORT MAX_SIZE_OF_RECV_PACKET{ 80 };	//sizeof(PACKET_DATA::CLIENT_TO_SERVER::Chat);	// (2) Recv 시, 처리해야하는 패킷 중 가장 큰 사이즈
	constexpr USHORT MAX_SIZE_OF_SEND{ 5 }; // sizeof(PACKET_DATA::SERVER_TO_CLIENT::Position);	// (5) Send 시, 처리해야하는 패킷 중 가장 큰 사이즈
	constexpr USHORT MAX_NUMBER_OF_SEND_POOL{ 100 };
	//---
	constexpr BYTE MAX_CHAT_MASSAGE_SIZE { 80 };
}