#include "pch.h"

#include "MemoryUnit.h"
#include "ClientContUnit.h"

ZoneContUnit::ZoneContUnit()
{
	for (auto& iter : clientContArr)
	{
		iter.reserve(1000);
		iter.push_back(nullptr);
	}

	for (auto& iter : indexArr)
	{
		iter = 0;
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

	// 어짜피 락이야 ~~~ ㅠㅠㅠㅠㅠㅠㅠㅠㅠㅠ락안쓸수가없었어 ㅠㅠㅠㅠㅠ
	clientContArr[contIndex][indexArr[contIndex]/*.load()*/] = pClient;	// 클라이언트 컨테이너에 접속한 클라이언트 구조체를 넣어줌.
	pClient->contIndex = indexArr[contIndex];

	indexArr[contIndex].fetch_add(1);	//인덱스 값을 하나 증가시킴.

	wrLockArr[contIndex].unlock();	// -------------------------------- 0
}

void ZoneContUnit::Exit(SocketInfo* pClient)
{
	const BYTE contIndex = GetContHashKey(pClient->nickname[0]);

	wrLockArr[contIndex].lock();	//++++++++++++++++++++++++++++++++ 1

	const USHORT contEndIndex = indexArr[contIndex].load();

	// 컨테이너 맨 뒤의 멤버를, 지워지는 멤버의 인덱스로 변경
	clientContArr[contIndex][pClient->contIndex] = clientContArr[contIndex][contEndIndex];

	// 맨뒤의 인덱스였던 애에게, 새로운 인덱스를 알려줌.
	clientContArr[contIndex][pClient->contIndex]->contIndex = pClient->contIndex;

	// 맨뒤의 컨테이너 제거 -> 해당 컨테이너의 허용 인덱스 변경.
	//clientContArr[contIndex].pop_back();
	indexArr[contIndex].fetch_sub(1);

	wrLockArr[contIndex].unlock();	// -------------------------------- 0
}

