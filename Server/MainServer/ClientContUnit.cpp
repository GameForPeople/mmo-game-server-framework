#include "pch.h"

#include "MemoryUnit.h"
#include "ServerDefine.h"
#include "ObjectInfo.h"
#include "BaseMonster.h"
#include "BaseNPC.h"

#include "ClientContUnit.h"

ZoneContUnit::ZoneContUnit() :
	clientContArr(),
	//lockArr(),
	//indexArr(),
	monsterCont(),
	npcCont()
{
	for (int i = 0 ; i < GLOBAL_DEFINE::MAX_CLIENT; ++i)
	{
		clientContArr[i] = new SocketInfo(i /* == Key */);
	}



	std::cout << "#. 접속 가능한 최대 플레이어는 " << GLOBAL_DEFINE::MAX_CLIENT << "입니다. \n";

	// Init Spin Lock 
	//for (auto& iter : lockArr)
	//{
	//	iter.clear(std::memory_order_release);
	//}
}

ZoneContUnit::~ZoneContUnit()
{
	// GoodBye Spin Lock 
	//for (auto& iter : lockArr)
	//{
	//	iter.clear(std::memory_order_release);
	//}

	for (auto& iter : clientContArr)
		delete iter;

	for (auto& iter : monsterCont)
		delete iter;

	//for (auto& iter : npcCont)
	//	delete iter;
}

//void ZoneContUnit::Enter(SocketInfo* pClient)
//{
	//const BYTE contHashIndex = GetContHashKey(pClient->objectInfo->key);
	//while (lockArr[contHashIndex].test_and_set(std::memory_order_acquire))  // acquire lock //++++++++++++++++++++++++++++++++ 1
	//	; // spin
	// lockArr[contHashIndex].clear(std::memory_order_release);               // release lock //+-------------------------------- 0
//}

//void ZoneContUnit::Exit(SocketInfo* pClient)
//{
	//const BYTE contHashIndex = GetContHashKey(pClient->key);
	//
	//while (lockArr[contHashIndex].test_and_set(std::memory_order_acquire))  // acquire lock //++++++++++++++++++++++++++++++++ 1
	//	; // spin
	//
	//const USHORT contEndIndex = indexArr[contHashIndex].load() - 1;
	//
	//// 컨테이너 맨 뒤의 멤버를, 지워지는 멤버의 인덱스로 변경
	//clientContArr[contHashIndex][pClient->contIndex] = clientContArr[contHashIndex][contEndIndex];
	//
	//// 맨뒤의 인덱스였던 애에게, 새로운 인덱스를 알려줌.
	//clientContArr[contHashIndex][pClient->contIndex]->contIndex = pClient->contIndex;
	//
	//// 맨뒤의 컨테이너 제거 -> 해당 컨테이너의 허용 인덱스 변경.
	////clientContArr[contIndex].pop_back();
	//indexArr[contHashIndex].fetch_sub(1);
	//
	//lockArr[contHashIndex].clear(std::memory_order_release);               // release lock //+-------------------------------- 0
//}

//std::pair<bool, SocketInfo*> ZoneContUnit::FindClient(_ClientKeyType inClientKey)
//{
//	const BYTE contHashIndex = GetContHashKey(inClientKey);
//	
//	for (auto iter : clientContArr[contHashIndex])
//	{
//		if (iter->objectInfo->key == inClientKey)
//		{
//			return std::make_pair(true, iter);
//		}
//	}
//
//	return std::make_pair(false, nullptr);
//}
