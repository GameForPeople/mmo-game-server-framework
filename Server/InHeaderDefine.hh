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