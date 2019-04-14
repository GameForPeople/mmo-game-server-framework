#include "pch.h"
#include "Define.h"
#include "ServerDefine.h"

#include "MemoryUnit.h"
#include "UserData.h"
#include "Zone.h"

#include "ClientContUnit.h"

#include "ConnectManager.h"

/*
	ConnectManager::InNewClient()
		- 새로운 클라이언트가 접속했을 떄, 이를 컨테이너에 넣어줍니다.

	#!?0. 하나의 물리 서버에서 하나의 씐을 가질 경우, 지금처럼하는게 맞음.
	#!?1. 다만 하나의 서버에서 여러 씐을 가질 경우, 애초에 SocketInfo를 갖고 있고, InNewCliet에 인자로 넣어주는 게맞음.
*/
_ClientNode ConnectManager::InNewClient(ZoneContUnit* inClientContUnit, Zone* zone)
{
	//std::lock_guard<std::mutex> localLock(addLock);
	//connectLock.lock();
	inClientContUnit->wrlock.lock();
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++1

	for (int index = 0; index < inClientContUnit->clientCont.size(); ++index)
	{
		if (inClientContUnit->clientCont[index].first == false)
		{
			inClientContUnit->clientCont[index].first = true;

			inClientContUnit->wrlock.unlock();
			//------------------------------------------------------------0

			// 소켓 정보 구조체 할당
			SocketInfo* pInClient = new SocketInfo;
			if (pInClient == nullptr)
			{
				ERROR_HANDLING::ERROR_QUIT(TEXT("Make_SocketInfo()"));
				break;
			}

			inClientContUnit->clientCont[index].second = pInClient;
			pInClient->clientKey = index;
			pInClient->pZone = zone;

			//SendPutPlayer(pInClient, inClientContUnit);

			return std::make_pair(true, pInClient);
			//return pClient;
		}
	}

	inClientContUnit->wrlock.unlock();
	//-------------------------------------------------------------------0
	return std::make_pair(false, nullptr);
	//return {};
}

/*
	ConnectManager::OutClient()
		- 클라이언트가 다른 레벨로 이동하거나, 로그아웃 될때, 해당 클라이언트를 레벨에서 빼줍니다.

	#!?0. 도대체 여기서 어디까지 보장을 해줘야하는건지. 이 보장이 오히려 버그가 될 수 있지 않은지.
*/
void ConnectManager::OutClient(SocketInfo* pOutClient, ZoneContUnit* inClientContUnit)
{
	// 사실 벡터면 굳이 Lock 걸 필요 없지 않나. -> 그래도 걸자........나는 찐따니까...
	SendRemovePlayer(pOutClient, inClientContUnit);

	inClientContUnit->clientCont[pOutClient->clientKey].first = false;
	// second는 초기화 할 필요 없음.

	// 다만 이부분에서, 비용이 조금 더 나가더라도, 안정성을 보장하기 위해 처리해주도록 합시다.
	// 사실 pZone을 nullptr하고, LogOutProcess에서, 해당 여부만 검사하는 것도 날 듯 한데;
	pOutClient->pZone = nullptr;
	pOutClient->clientKey = -1;
}

// 더 이상 해당 함수는 ConnectManager에서 실행되지 않습니다. Sector로 변경.
//void ConnectManager::SendPutPlayer(SocketInfo* pPutClient, ZoneContUnit* inClientCont)
//{
//	PACKET_DATA::SC::PutPlayer packet(
//		pPutClient->clientKey,
//		pPutClient->userData->GetPosition().x,
//		pPutClient->userData->GetPosition().y
//	);
//
//	for (std::pair<bool, SocketInfo*>& pRecvedClient : inClientCont->clientCont)
//	{
//		if (pRecvedClient.first)
//		{
//			NETWORK_UTIL::SendPacket(pRecvedClient.second, reinterpret_cast<char*>(&packet));
//		}
//	}
//}
//
void ConnectManager::SendRemovePlayer(SocketInfo* pOutClient, ZoneContUnit* inClientCont)
{
	PACKET_DATA::SC::RemovePlayer packet(
		pOutClient->clientKey
	);

	inClientCont->wrlock.lock_shared(); // +++++++++++++++++++++++++++++++++++++++++++++++++++1 read
	for (auto otherKey : pOutClient->viewList)
	{
		NETWORK_UTIL::SendPacket(inClientCont->clientCont[otherKey].second, reinterpret_cast<char*>(&packet));
		
		// 상대방 viewList 수정.
		inClientCont->clientCont[otherKey].second->viewList.unsafe_erase(pOutClient->clientKey);
		// 나의 viewList는 건들필요 없음.
	}
	inClientCont->wrlock.unlock_shared(); //--------------------------------------------------0 read

	//for (std::pair<bool, SocketInfo*>& pRecvedClient : inClientCont)
	//{
	//	if (pRecvedClient.first)
	//	{
	//		NETWORK_UTIL::SendPacket(pRecvedClient.second, reinterpret_cast<char*>(&packet));
	//	}
	//}
}