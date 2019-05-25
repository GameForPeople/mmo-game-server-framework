#pragma once

#pragma once
/*
	InHeaderDefine

	!0. 해당 헤더는, 유일하게 다른 헤더 파일에 포함되는 헤더입니다.
*/

#define _DEV_MODE_

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

using _PosType = unsigned short;
using _HpType = unsigned short;
using _MpType = unsigned short;
using _LevelType = unsigned char;
using _DamageType = unsigned short;
using _StateType = unsigned char;

using _SectorIndexType = unsigned char;

using _TickCountType = unsigned char;

enum class OBJECT_TYPE : unsigned char
{
	PLAYER,
	MONSTER,
	NPC
};
