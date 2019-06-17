#include "pch.h"
#include "../Define.h"

#include "MemoryUnit.h"
#include "SendMemoryPool.h"
#include "Zone.h"
#include "ObjectInfo.h"
#include "ServerDefine.h"
#include "BaseMonster.h"
#include "LuaManager.h"

namespace NETWORK_UTIL
{
	extern SOCKET querySocket;
	extern QueryMemoryUnit* queryMemoryUnit;
	/*
		SendPacket()
			- WSASend!는 여기에서만 존재할 수 있습니다.

		!0. 단순히 WSA Send만 있는 함수입니다. 데이터는 준비해주세요.
		!1. 이 함수를 호출하기 전에, wsaBuf의 len을 설정해주세요.

		?0. wsaBuf의 buf는 보낼때마다 바꿔줘야 할까요?
	*/
	void SendPacket(SocketInfo* pClient, char* packetData)
	{
		SendMemoryUnit* sendMemoryUnit = SendMemoryPool::GetInstance()->PopMemory();
		memcpy(sendMemoryUnit->memoryUnit.dataBuf, packetData, packetData[0]);
		
		sendMemoryUnit->memoryUnit.wsaBuf.len = static_cast<ULONG>(packetData[0]);

#ifdef _DEV_MODE_
		//std::cout << "길이 : " << sendMemoryUnit->memoryUnit.wsaBuf.len << "타입 : " << (int)packetData[1] << "내용 : " << (int)packetData[2];
#endif

		ZeroMemory(&(sendMemoryUnit->memoryUnit.overlapped), sizeof(sendMemoryUnit->memoryUnit.overlapped));

        //ERROR_HANDLING::errorRecvOrSendArr[
		//	static_cast<bool>(
				//1 + 
		if (SOCKET_ERROR == WSASend(pClient->sock, &(sendMemoryUnit->memoryUnit.wsaBuf), 1, NULL, 0, &(sendMemoryUnit->memoryUnit.overlapped), NULL) )
		{
			if (ERROR_HANDLING::HandleSendError())
			{
				LuaManager::GetInstance()->pZone->Death(pClient);
				closesocket(pClient->sock);
				SendMemoryPool::GetInstance()->PushMemory(sendMemoryUnit);
			}
		}
			//]();
	}

	void SendQueryPacket(char* packetData)
	{
		SendMemoryUnit* sendMemoryUnit = SendMemoryPool::GetInstance()->PopMemory();
		memcpy(sendMemoryUnit->memoryUnit.dataBuf, packetData, packetData[0]);

		sendMemoryUnit->memoryUnit.wsaBuf.len = static_cast<ULONG>(packetData[0]);
		
		ZeroMemory(&(sendMemoryUnit->memoryUnit.overlapped), sizeof(sendMemoryUnit->memoryUnit.overlapped));

		if (SOCKET_ERROR ==
			WSASend(querySocket, &(sendMemoryUnit->memoryUnit.wsaBuf), 1, NULL, 0, &(sendMemoryUnit->memoryUnit.overlapped), NULL)
			)
		{
			if (auto retValue = WSAGetLastError(); 
				retValue == ERROR_IO_PENDING)
			{
				return;
			}
			ERROR_HANDLING::ERROR_DISPLAY(L"SendQuery()");
		}
	}

#ifndef DISABLED_UNALLOCATED_MEMORY_SEND
	void SendUnallocatedPacket(SocketInfo* pClient, char* pOriginData)
	{
		UnallocatedMemoryUnit* pUnallocatedMemoryUnit
			= SendMemoryPool::GetInstance()->PopUnallocatedMemory();

		pUnallocatedMemoryUnit->wsaBuf.buf = pOriginData;
		pUnallocatedMemoryUnit->wsaBuf.len = static_cast<ULONG>(pOriginData[0]);

		ZeroMemory(&(pUnallocatedMemoryUnit->overlapped), sizeof(pUnallocatedMemoryUnit->overlapped));

		if (SOCKET_ERROR ==
			WSASend(pClient->sock, &(pUnallocatedMemoryUnit->wsaBuf), 1, NULL, 0, &(pUnallocatedMemoryUnit->overlapped), NULL)
			)
		{
			ERROR_HANDLING::ERROR_DISPLAY("SendUnallocatedPacket()");
		}
	}
#endif

	/*
		RecvPacket()
			- WSA Recv는 여기서만 존재합니다.

		!0. SocketInfo에 있는 wsaBuf -> buf 에 리시브를 합니다. 
		!1. len은 고정된 값을 사용합니다. MAX_SIZE_OF_RECV_BUFFER!
	*/
	void RecvPacket(SocketInfo* pClient)
	{
		// 받은 데이터에 대한 처리가 끝나면 바로 다시 받을 준비.
		DWORD flag{};

		ZeroMemory(&(pClient->memoryUnit.overlapped), sizeof(pClient->memoryUnit.overlapped));

		//ERROR_HANDLING::errorRecvOrSendArr[
		//	static_cast<bool>(
				//1 + 
		if (SOCKET_ERROR == WSARecv(pClient->sock, &(pClient->memoryUnit.wsaBuf), 1, NULL, &flag /* NULL*/, &(pClient->memoryUnit.overlapped), NULL))
		{
			ERROR_HANDLING::HandleRecvError();
			//ERROR_HANDLING::ERROR_DISPLAY("못받았어요....");
		}
		//		)
		//]();
	}

	void RecvQueryPacket()
	{
		DWORD flag{};
		ZeroMemory(&(queryMemoryUnit->memoryUnit.overlapped), sizeof(queryMemoryUnit->memoryUnit.overlapped));
		if (SOCKET_ERROR == WSARecv(querySocket, &(queryMemoryUnit->memoryUnit.wsaBuf), 1, NULL, &flag /* NULL*/, &(queryMemoryUnit->memoryUnit.overlapped), NULL))
		{
			ERROR_HANDLING::HandleRecvError();
			//ERROR_HANDLING::ERROR_DISPLAY("못받았어요....");
		}
	}

	namespace SEND
	{
		void SendRemovePlayer(const _KeyType pRemoveClientID, SocketInfo* pRecvClient)
		{
			PACKET_DATA::MAIN_TO_CLIENT::RemovePlayer packet(pRemoveClientID);

			NETWORK_UTIL::SendPacket(pRecvClient, reinterpret_cast<char*>(&packet));
		}

		void SendChatMessage(const WCHAR* message, const _KeyType senderKey, SocketInfo* pRecvClient)
		{
			PACKET_DATA::MAIN_TO_CLIENT::Chat packet(senderKey, message);
			NETWORK_UTIL::SendPacket(pRecvClient, reinterpret_cast<char*>(&packet));
		}

		void SendStatChange(const char inStatChangeType, const int inNewValue, SocketInfo* pRecvClient)
		{
			PACKET_DATA::MAIN_TO_CLIENT::StatChange packet(inStatChangeType, inNewValue);
			NETWORK_UTIL::SendPacket(pRecvClient, reinterpret_cast<char*>(&packet));
		}
	}
	/*
		LogOutProcess()
			- 클라이언트의 로그아웃 처리를 합니다.

			#0. OutClient 함수에서, pZone의 nullptr와 clientKey의 -1을 보장합니다.

			!0. 다만, Socket이 없는 경우 해당 함수 호출 시, 문제가 될 수 있습니다. (Accept 실패 시 
			!1. 또, nullptr가 인자로 들어올 경우, nullptr 참조 오류가 발생합니다.
			!2. pClient의 pZone이 nullptr일 경우, nullptr 참조 오류가 발생합니다. 

			#0. 성능상의 이슈로, !0, !1, !2의 nullptr여부를 보장하지 않습니다. ( 적합한 구조일 경우, nullptr참조가 발생하기 어려움 )
	*/

//	void LogOutProcess(LPVOID pClient)
//	{
//
//		//if (reinterpret_cast<MemoryUnit*>(pClient)->memoryUnitType == MEMORY_UNIT_TYPE::RECV_FROM_CLIENT)
//		//{
//		SocketInfo* pOutClient = reinterpret_cast<SocketInfo*>(pClient);
//
//		SOCKADDR_IN clientAddr;
//
//		int addrLength = sizeof(clientAddr);
//
//		getpeername(pOutClient->sock, (SOCKADDR*)& clientAddr, &addrLength);
//		std::cout << " [GOODBYE] 클라이언트 (" << inet_ntoa(clientAddr.sin_addr) << ") 가 종료했습니다. \n";
//
//		// 애초에 존에 접속도 못했는데, 로그아웃 할 경우를 방지.
//		if (pOutClient->key != -1) pOutClient->Exit(pOutClient);
//
//		closesocket(pOutClient->sock);
//		delete pOutClient;
//		//}
//		//else if (reinterpret_cast<MemoryUnit*>(pClient)->memoryUnitType == MEMORY_UNIT_TYPE::SEND_TO_CLIENT)
//		//{
//		//	SendMemoryUnit* pMemoryUnit = (reinterpret_cast<SendMemoryUnit*>(pClient));
//		//	SendMemoryPool::GetInstance()->PushMemory(pMemoryUnit);
//		//}
//#ifndef DISABLED_UNALLOCATED_MEMORY_SEND
//		else if (reinterpret_cast<MemoryUnit*>(pClient)->memoryUnitType == MEMORY_UNIT_TYPE::SEND)
//		{
//			UnallocatedMemoryUnit* pMemoryUnit = (reinterpret_cast<UnallocatedMemoryUnit*>(pClient));
//
//			SendMemoryPool::GetInstance()->PushUnallocatedMemory(pMemoryUnit);
//		}
//#endif
//	}

}

namespace ERROR_HANDLING
{
	/*
		ERROR_QUIT
			- 서버에 심각한 오류가 발생할 경우, 메세지 박스를 활용해 에러를 출력하고, 서버를 종료합니다.
	*/
	_NORETURN void ERROR_QUIT(const WCHAR *msg)
	{
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			WSAGetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0,
			NULL
		);

		MessageBox(NULL, (LPTSTR)lpMsgBuf, msg, MB_ICONERROR);
		LocalFree(lpMsgBuf);
		exit(1);
	};

	/*
		ERROR_DISPLAY
			- 서버에 오류가 발생할 경우, 에러 로그를 출력합니다.
	*/
	/*_DEPRECATED*/ void ERROR_DISPLAY(const WCHAR *msg)
	{
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, WSAGetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf, 0, NULL);
		wprintf(L"[%s] %s", msg, (WCHAR*)lpMsgBuf);
		LocalFree(lpMsgBuf);
	};

	/*
		HandleRecvOrSendError
			- Send Or Recv 실패 시 출력되는 함수로, 에러를 출력합니다.
	*/
	void HandleRecvError()
	{
		auto retValue = WSAGetLastError();

		if (retValue == ERROR_IO_PENDING)
		{
			return;
		}
		if (retValue == WSAECONNRESET)
		{
			return;
		}
		if (retValue == WSAENOTSOCK)
		{
			return;
		}

		ERROR_DISPLAY(((L"HandleRecvError() : " + std::to_wstring(retValue)).c_str()));
	}

	bool HandleSendError()
	{
		auto retValue = WSAGetLastError();

		if (retValue == ERROR_IO_PENDING)
		{
			return false;
		}
		if (retValue == WSAECONNRESET)
		{
			return true;
		}
		if (retValue == WSAENOTSOCK)
		{
			return true;
		}
		
		ERROR_DISPLAY(((L"HandleSendError() : " + std::to_wstring(retValue)).c_str()));
	}
}

namespace JOB
{
	unsigned short GetMaxHP(_JobType inJob, unsigned char inLevel) noexcept
	{
		unsigned short  maxHp = BASE_HP; // 기본 체력

		if (inJob == JOB_TYPE::KNIGHT)
		{
			maxHp = maxHp + (inLevel * KNIGHT_HP_PER_LEVEL);
		}
		else if (inJob == JOB_TYPE::ARCHER)
		{
			maxHp = maxHp + (inLevel * ARCHER_HP_PER_LEVEL);
		}
		else if (inJob == JOB_TYPE::WITCH)
		{
			maxHp = maxHp + (inLevel * WITCH_HP_PER_LEVEL);
		}

		return maxHp;
	}

	unsigned short GetMaxMP(_JobType inJob, unsigned char inLevel) noexcept
	{
		unsigned short  maxMp = BASE_MP; // 기본 마나

		if (inJob == JOB_TYPE::KNIGHT)
		{
			maxMp = maxMp + (inLevel * KNIGHT_MP_PER_LEVEL);
		}
		else if (inJob == JOB_TYPE::ARCHER)
		{
			maxMp = maxMp + (inLevel * ARCHER_MP_PER_LEVEL);
		}
		else if (inJob == JOB_TYPE::WITCH)
		{
			maxMp = maxMp + (inLevel * WITCH_MP_PER_LEVEL);
		}

		return maxMp;
	}

	unsigned short GetDamage(_JobType inJob, /*_LevelType*/ unsigned char inLevel) noexcept
	{
		unsigned short retDamage = BASE_DAMAGE; // 기본 마나

		if (inJob == JOB_TYPE::KNIGHT)
		{
			retDamage = retDamage + (inLevel * KNIGHT_DAMAGE_PER_LEVEL);
		}
		else if (inJob == JOB_TYPE::ARCHER)
		{
			retDamage = retDamage + (inLevel * ARCHER_DAMAGE_PER_LEVEL);
		}
		else if (inJob == JOB_TYPE::WITCH)
		{
			retDamage = retDamage + (inLevel * WITCH_DAMAGE_PER_LEVEL);
		}
		return retDamage;
	}

	bool IsInAttackRange(int range, SocketInfo* pHitter, BaseMonster* pMonster)
	{
		return (range >= abs(pHitter->objectInfo->posX - pMonster->objectInfo->posX) + abs(pHitter->objectInfo->posY - pMonster->objectInfo->posY));
	}

	bool IsAttack(_JobType inJob, unsigned char inAttackkType, SocketInfo* pClient, BaseMonster* pMonster) noexcept
	{
		if (inJob == JOB_TYPE::KNIGHT)
		{
			if (inAttackkType == 0)
			{
				return IsInAttackRange(KNIGHT_ATTACK_0_RANGE, pClient, pMonster);
			}
			else if (inAttackkType == 2)
			{
				return IsInAttackRange(KNIGHT_ATTACK_2_RANGE, pClient, pMonster);
			}
		}
		else if (inJob == JOB_TYPE::ARCHER)
		{
			// 모두 동일함
			if (inAttackkType == 0 || inAttackkType == 1 || inAttackkType == 2)
			{
				return IsInAttackRange(ARCHER_ATTACK_0_RANGE, pClient, pMonster);
			}
		}
		else if (inJob == JOB_TYPE::WITCH)
		{
			if (inAttackkType == 0)
			{
				return IsInAttackRange(WITCH_ATTACK_0_RANGE, pClient, pMonster);
			}
			else if (inAttackkType == 1)
			{
				return IsInAttackRange(WITCH_ATTACK_1_RANGE, pClient, pMonster);
			}
			else if (inAttackkType == 2)
			{
				return IsInAttackRange(WITCH_ATTACK_2_RANGE, pClient, pMonster);
			}
		}
		return false;
	}
}

namespace LUA_UTIL
{
	void PrintError(lua_State* L)
	{
		std::cout << lua_tostring(L, -1);
		lua_pop(L, -1);
	}
}

namespace TIME_UTIL {
	const std::string GetCurrentDateTime() {
		time_t     now = time(0); //현재 시간을 time_t 타입으로 저장
		struct tm  tstruct;
		char       buf[80];
		localtime_s(&tstruct, &now);
		strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct); // YYYY-MM-DD.HH:mm:ss 형태의 스트링

		return buf;
	}
}
