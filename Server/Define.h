#pragma once

/*
	Define.h
		- 해당 헤더 파일은, 서버와 클라이언트가 공통으로 사용합니다. 
		(클라이언트에서 서버 프로젝트 코드 참조)

	#0. 해당 헤더는, Enum, Static Constexp
*/

namespace GLOBAL_DEFINE
{
	constexpr USHORT SERVER_PORT = 9000;
}

namespace PACKET_TYPE
{
	enum /* class PACKET_TYPE : BYTE */
	{
		MOVE /* = 0*/,
		ENUM_SIZE
	};
}

namespace NETWORK_TYPE
{
	enum NETWORK_TYPE /*class NETWORK_TYPE : BYTE */
	{
		RECV /* = 0*/,
		SEND,
		ENUM_SIZE
	};
}