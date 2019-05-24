#pragma once

#include "InHeaderDefine.hh"

/*
	MonsterLoader
	
		- 서버를 로드할 때, File I/O를 통해 몬스터 정보를 로드하고,
			바로 제거되는(필요가 없는) 객체입니다.
*/

class MonsterLoader
{
	using _PosContType = std::vector<std::pair<_PosType, _PosType>>;

public:
	MonsterLoader();

	_NODISCARD inline _PosContType& GetSlimePosCont() noexcept { return slimePosCont; };
	_NODISCARD inline _PosContType& GetGolemPosCont() noexcept { return golemPosCont; };

private:
	_PosContType slimePosCont;
	_PosContType golemPosCont;
};