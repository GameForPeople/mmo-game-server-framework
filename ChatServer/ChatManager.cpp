#include "pch.h"
#include "Define.h"
#include "ServerDefine.h"
#include "MemoryUnit.h"
#include "ClientContUnit.h"
#include "ChatManager.h"

ChatManager::ChatManager()
	: sendedMessageCont()
{
	sendedMessageCont.clear();

	for(int i = 0 ; i < ChatUnit::chatUnitCount; ++i)
		sendedMessageCont.push(new ChatUnit());
}

ChatManager::~ChatManager()
{
	ChatUnit* pChatUnit = nullptr;
	while (sendedMessageCont.try_pop(pChatUnit))
	{
		delete pChatUnit;
	}
}

/*
	ChatProcess
		- 채팅 메세지가 날라오면, 해당 내용 컨테이너에 저장하고, 존 유저들에게 브로드캐스팅
	
	!0. 1차 오바입니다. -> PZoneCont에다가 락을 걸어버렸습니다.
	!1. 2차 오바입니다. -> 락을 건 상태로 모든 클라이언트를 순회합니다.
	!2. 3차 오바입니다. -> 그 내부에서 Send함수까지 호출했습니다.

	삼진오바로 리팩토링이 가결되었습니다.

	[19-04-29]
		- 리팩토링을 가결하였습니다. 오바를 다 날려버렸습니다.
*/
void ChatManager::ChatProcess(SocketInfo* pClient, ZoneContUnit* pZoneContUnit)
{
#ifdef UNALLOCATED_SEND
	ChatUnit* retChatUnit{ nullptr };
	while (!sendedMessageCont.try_pop(retChatUnit))
	{
		ERROR_HANDLING::ERROR_DISPLAY(L"[ERROR]SendPool의 메모리가 부족합니다.");
		/*
			원래는 여기서 메모리 추가로 할당해서, 넘겨줘야해 어딜 기다려!
		*/
	}
	// 받은 데이터값 그대로 사용합니다.
	// memcpy(retChatUnit->message, pClient->loadedBuf, pClient->loadedBuf[0]);

	//pZoneContUnit->wrlock.lock_shared(); //++++++++++++++++++++++++++++++++++1
	//for (auto& iter : pZoneContUnit->clientCont)
	//{
	//	NETWORK_UTIL::SendUnallocatedPacket(iter.second, reinterpret_cast<char*>(retChatUnit));
	//}
	//pZoneContUnit->wrlock.unlock_shared(); //--------------------------------0
	//
	//sendedMessageCont.push(retChatUnit);
#endif
	PACKET_DATA::CHAT_SERVER_TO_CLIENT::Chat packet(pClient->loadedBuf);

	BYTE arrIndex = 0;
	for (auto& iter : pZoneContUnit->clientContArr)
	{
		for (int i = 0; i < pZoneContUnit->indexArr[arrIndex]; ++i)
		{
			NETWORK_UTIL::SendPacket(iter[i], reinterpret_cast<char*>(&packet));
		}
		++arrIndex;
	}
}