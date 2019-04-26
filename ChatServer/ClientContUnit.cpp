#include "pch.h"

#include "MemoryUnit.h"

#include "ClientContUnit.h"

ZoneContUnit::ZoneContUnit()
{
	for (auto& iter : clientContArr)
	{
		iter.reserve(1000);
	}
}

ZoneContUnit::~ZoneContUnit()
{
	for (auto& iter : clientContArr)
	{
		for (auto& pSocketInfo : iter)
		{
			delete pSocketInfo;
		}
	}
}


void ZoneContUnit::Enter(SocketInfo* pClient)
{
	const BYTE contIndex = GetContHashKey(pClient->nickname[0]);

	wrLockArr[contIndex].lock();	//++++++++++++++++++++++++++++++++ 1

	clientContArr[contIndex].emplace_back(pClient);
	pClient->contIndex = clientContArr[contIndex].size() - 1;

	wrLockArr[contIndex].unlock();	// -------------------------------- 0
}

void ZoneContUnit::Exit(SocketInfo* pClient)
{
	const BYTE contIndex = GetContHashKey(pClient->nickname[0]);

	wrLockArr[contIndex].lock();	//++++++++++++++++++++++++++++++++ 1

	const USHORT contEndIndex = clientContArr[contIndex].size() - 1;

	// 컨테이너 맨 뒤의 멤버를, 지워지는 멤버의 인덱스로 변경
	clientContArr[contIndex][pClient->contIndex] = clientContArr[contIndex][contEndIndex];

	// 맨뒤의 인덱스였던 애에게, 새로운 인덱스를 알려줌.
	clientContArr[contIndex][pClient->contIndex]->contIndex = pClient->contIndex;

	// 맨뒤의 컨테이너 제거
	clientContArr[contIndex].pop_back();

	wrLockArr[contIndex].unlock();	// -------------------------------- 0
}

