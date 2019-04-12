#include "pch.h"
#include "MemoryUnit.h"
#include "SendMemoryPool.h"
#include "Scene.h"
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
		SendMemoryUnit* sendMemoryUnit = SendMemoryPool::GetInstance()->PopMemory(pClient);
		memcpy(sendMemoryUnit->memoryUnit.dataBuf, packetData, packetData[0]);
		
		sendMemoryUnit->memoryUnit.wsaBuf.len = packetData[0];

		DWORD flag{};
		ZeroMemory(&sendMemoryUnit->memoryUnit.overlapped, sizeof(sendMemoryUnit->memoryUnit.overlapped));

		ERROR_HANDLING::errorRecvOrSendArr[
			static_cast<bool>(
				1 + WSASend(pClient->sock, &sendMemoryUnit->memoryUnit.wsaBuf, 1, NULL, 0, &sendMemoryUnit->memoryUnit.overlapped, NULL)
				)
		]();
	}

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

		ZeroMemory(&pClient->memoryUnit.overlapped, sizeof(pClient->memoryUnit.overlapped));

		ERROR_HANDLING::errorRecvOrSendArr[
			static_cast<bool>(
				1 + WSARecv(pClient->sock, &pClient->memoryUnit.wsaBuf, 1, NULL, &flag /* NULL*/, &pClient->memoryUnit.overlapped, NULL)
				)
		]();
	}

	/*
		LogOutProcess()
			- 클라이언트의 로그아웃 처리를 합니다.

			#0. OutClient 함수에서, pScene의 nullptr와 clientContIndex의 -1을 보장합니다.

			!0. 다만, Socket이 없는 경우 해당 함수 호출 시, 문제가 될 수 있습니다. (Accept 실패 시 
			!1. 또, nullptr가 인자로 들어올 경우, nullptr 참조 오류가 발생합니다.
			!2. pClient의 pScene이 nullptr일 경우, nullptr 참조 오류가 발생합니다. 

			#0. 성능상의 이슈로, !0, !1, !2의 nullptr여부를 보장하지 않습니다. ( 적합한 구조일 경우, nullptr참조가 발생하기 어려움 )
	*/
	void LogOutProcess(MemoryUnit* pClient)
	{
		SocketInfo* pOutClient;
		if (pClient->isRecv)
		{
			pOutClient = reinterpret_cast<SocketInfo*>(pClient);

			SOCKADDR_IN clientAddr;

			int addrLength = sizeof(clientAddr);

			getpeername(pOutClient->sock, (SOCKADDR*)& clientAddr, &addrLength);
			std::cout << " [GOODBYE] 클라이언트 (" << inet_ntoa(clientAddr.sin_addr) << ") 가 종료했습니다. \n";
			if (pOutClient->clientContIndex != -1) pOutClient->pScene->OutClient(pOutClient);

			closesocket(pOutClient->sock);
			delete pOutClient;
		}
		else
		{
			SendMemoryUnit* pMemoryUnit = (reinterpret_cast<SendMemoryUnit*>(pClient));
			
			// Send 예외처리 Off
			//pOutClient = pMemoryUnit->pOwner;
			SendMemoryPool::GetInstance()->PushMemory(pMemoryUnit);
		}
	}

}

namespace BIT_CONVERTER
{
	/*
		MakeByteFromLeftAndRightByte()
			- 인자로 들어온 패킷 타입(바이트)의 최상위 비트를 1로 바꾼후 반환합니다.

		!0. 패킷 타입 개수가, ox7f보다 클 경우, 해당 함수 및 현재 서버 로직은 오류가 발생합니다.
	*/
	_NODISCARD BYTE MakeSendPacket(const BYTE inPacketType) noexcept { return inPacketType | SEND_BYTE; }

	/*
		MakeByteFromLeftAndRightByte()
			- 인자로 들어온 패킷 타입(바이트)의 최상위 비트를 검사합니다.

		#0. 최상위 비트가 1일 경우, true를, 0일 경우 false를 반환합니다. (형변환을 통한 Array Overflow 방지 )

		!0. 패킷 타입 개수가, ox7f보다 클 경우, 해당 함수 및 현재 서버 로직은 오류가 발생합니다.
	*/
	_NODISCARD bool GetRecvOrSend(const char inChar) noexcept { return (inChar >> 7) & (0x01); }
	
	/*
		MakeByteFromLeftAndRightByte
			- 0x10보다 작은 바이트 두개를 인자로 받아(Left, Right) 상위 4개비트는 Left, 하위 4개 비트는 Right를 담아반환합니다.

		!0. 인자로 들어오는 두 바이트 모두, 0x10보다 작은 값이 들어와야합니다.
			- 인자로 들어오는 Left 바이트가 0x0f보다 큰 값이 들어올 경우, 오버플로우되어 비정상적인 값이 반횐될 수 있음.
			- 인자로 들어오는 Right 바이트가 0x0f보다 큰 값이 들어올 경우, LeftByte의 | 연산에서 비정상적인 값을 반환할 수 있음.
	*/
	_NODISCARD BYTE MakeByteFromLeftAndRightByte(const BYTE inLeftByte, const BYTE inRightByte) noexcept
	{
		return (inLeftByte << 4) | (inRightByte);
	}

	/*
		GetLeft4Bit
			- HIWORD(?)와 유사하게 동작합니다. 하나의 바이트를 받아서 상위(좌측) 4개의 비트를 바이트로 변환해서 반환합니다.
	*/
	_NODISCARD BYTE GetLeft4Bit(const BYTE inByte) noexcept
	{
		return (inByte >> 4) & (0x0f);
	}

	/*
		GetRight4Bit
			- LOWORD(?)와 유사하게 동작합니다. 하나의 바이트를 받아서 하위(우측) 4개의 비트를 바이트로 변환해서 반환합니다.
	*/
	_NODISCARD BYTE GetRight4Bit(const BYTE inByte) noexcept
	{
		return (inByte) & (0x0f);
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
	/*_DEPRECATED*/ void ERROR_DISPLAY(const CHAR *msg)
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
		std::wcout << L" Error no.%s" << msg << L" - " << lpMsgBuf;
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
			ERROR_DISPLAY(("RecvOrSend()"));
		}
	}
}

