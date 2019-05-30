#include "pch.h"
#include "../Define.h"

#include "MemoryUnit.h"
#include "SendMemoryPool.h"
#include "Zone.h"
#include "ObjectInfo.h"
#include "ServerDefine.h"

namespace NETWORK_UTIL
{
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
		std::cout << "길이 : " << sendMemoryUnit->memoryUnit.wsaBuf.len << "타입 : " << (int)packetData[1] << "내용 : " << (int)packetData[2];
#endif

		ZeroMemory(&(sendMemoryUnit->memoryUnit.overlapped), sizeof(sendMemoryUnit->memoryUnit.overlapped));

        //ERROR_HANDLING::errorRecvOrSendArr[
		//	static_cast<bool>(
				//1 + 
		if (SOCKET_ERROR ==
			WSASend(pClient->sock, &(sendMemoryUnit->memoryUnit.wsaBuf), 1, NULL, 0, &(sendMemoryUnit->memoryUnit.overlapped), NULL)
			)
		{
			ERROR_HANDLING::ERROR_DISPLAY(L"SendPacket()");
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
			ERROR_HANDLING::HandleRecvOrSendError();
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
			ERROR_HANDLING::HandleRecvOrSendError();
			//ERROR_HANDLING::ERROR_DISPLAY("못받았어요....");
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
	void LogOutProcess(LPVOID pClient)
	{
		if (reinterpret_cast<MemoryUnit*>(pClient)->memoryUnitType == MEMORY_UNIT_TYPE::RECV_FROM_CLIENT)
		{
			SocketInfo* pOutClient = reinterpret_cast<SocketInfo*>(pClient);

			SOCKADDR_IN clientAddr;

			int addrLength = sizeof(clientAddr);

			getpeername(pOutClient->sock, (SOCKADDR*)& clientAddr, &addrLength);
			std::cout << " [GOODBYE] 클라이언트 (" << inet_ntoa(clientAddr.sin_addr) << ") 가 종료했습니다. \n";
			
			// 애초에 존에 접속도 못했는데, 로그아웃 할 경우를 방지.
			if (pOutClient->objectInfo->key != -1) pOutClient->pZone->Exit(pOutClient);

			closesocket(pOutClient->sock);
			delete pOutClient;
		}
		else if (reinterpret_cast<MemoryUnit*>(pClient)->memoryUnitType == MEMORY_UNIT_TYPE::SEND_TO_CLIENT)
		{
			SendMemoryUnit* pMemoryUnit = (reinterpret_cast<SendMemoryUnit*>(pClient));
			SendMemoryPool::GetInstance()->PushMemory(pMemoryUnit);
		}
#ifndef DISABLED_UNALLOCATED_MEMORY_SEND
		else if (reinterpret_cast<MemoryUnit*>(pClient)->memoryUnitType == MEMORY_UNIT_TYPE::SEND)
		{
			UnallocatedMemoryUnit* pMemoryUnit = (reinterpret_cast<UnallocatedMemoryUnit*>(pClient));

			SendMemoryPool::GetInstance()->PushUnallocatedMemory(pMemoryUnit);
		}
#endif
	}
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
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			WSAGetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0,
			NULL
		);

		//C603 형식 문자열이 일치하지 않습니다. 와이드 문자열이 _Param_(3)으로 전달되었습니다.
		//printf(" [%s]  %s", msg, (LPTSTR)&lpMsgBuf);
		std::wcout << L" Error no." << msg << L" - " << lpMsgBuf;
		LocalFree(lpMsgBuf);
	};

	/*
		HandleRecvOrSendError
			- Send Or Recv 실패 시 출력되는 함수로, 에러를 출력합니다.
	*/
	void HandleRecvOrSendError()
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			ERROR_DISPLAY((L"RecvOrSend()"));
		}
	}
}

