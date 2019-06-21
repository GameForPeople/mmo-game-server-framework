#pragma once

namespace NETWORK_UTIL
{
	namespace SEND
	{
		template <class OBJECT, class PACKET_TY>
		void SendPutPlayer(OBJECT* pPutObject, SocketInfo* pRecvClient)
		{
			/*PACKET_DATA::MAIN_TO_CLIENT::PutPlayer*/ PACKET_TY packet(
				pPutObject->key,
				//pPutClient->userData->GetPosition().x,
				//pPutClient->userData->GetPosition().y
				pPutObject->objectInfo->posX,
				pPutObject->objectInfo->posY,
				pPutObject->objectInfo->job
			);

			NETWORK_UTIL::SendPacket(pRecvClient, reinterpret_cast<char*>(&packet));
		}

		template <class OBJECT, class PACKET_POSITION>
		void SendMovePlayer(OBJECT* pMovedObject, SocketInfo* pRecvClient)
		{
			/*PACKET_DATA::MAIN_TO_CLIENT::Position*/ PACKET_POSITION packet(
				pMovedObject->key,
				//pMovedClientKey->userData->GetPosition().x,
				//pMovedClientKey->userData->GetPosition().y
				pMovedObject->objectInfo->posX,
				pMovedObject->objectInfo->posY
			);

			NETWORK_UTIL::SendPacket(pRecvClient, reinterpret_cast<char*>(&packet));
		}
	}
}

namespace ATOMIC_UTIL {
	template <class TYPE>
	bool T_CAS(std::atomic<TYPE> *addr, TYPE inOldValue, TYPE inNewValue)
	{
		return atomic_compare_exchange_strong(addr, &inOldValue, inNewValue);
	}
}