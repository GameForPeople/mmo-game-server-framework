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

	// 嬢促杷 喰戚醤 ~~~ ばばばばばばばばばば喰照承呪亜蒸醸嬢 ばばばばば
	clientContArr[contIndex][indexArr[contIndex]/*.load()*/] = pClient;	// 適虞戚情闘 珍砺戚格拭 羨紗廃 適虞戚情闘 姥繕端研 隔嬢捜.
	pClient->contIndex = indexArr[contIndex];

	indexArr[contIndex].fetch_add(1);	//昔畿什 葵聖 馬蟹 装亜獣鉄.

	wrLockArr[contIndex].unlock();	// -------------------------------- 0
}

void ZoneContUnit::Exit(SocketInfo* pClient)
{
	const BYTE contIndex = GetContHashKey(pClient->nickname[0]);

	wrLockArr[contIndex].lock();	//++++++++++++++++++++++++++++++++ 1

	const USHORT contEndIndex = indexArr[contIndex].load();

	// 珍砺戚格 固 及税 呉獄研, 走趨走澗 呉獄税 昔畿什稽 痕井
	clientContArr[contIndex][pClient->contIndex] = clientContArr[contIndex][contEndIndex];

	// 固及税 昔畿什心揮 蕉拭惟, 歯稽錘 昔畿什研 硝形捜.
	clientContArr[contIndex][pClient->contIndex]->contIndex = pClient->contIndex;

	// 固及税 珍砺戚格 薦暗 -> 背雁 珍砺戚格税 買遂 昔畿什 痕井.
	//clientContArr[contIndex].pop_back();
	indexArr[contIndex].fetch_sub(1);

	wrLockArr[contIndex].unlock();	// -------------------------------- 0
}

