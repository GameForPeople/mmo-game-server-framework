#include "pch.h"
#include "SocketInfo.h"
#include "SendMemoryPool.h"
#include "Scene.h"
#include "ServerDefine.h"

namespace NETWORK_UTIL
{
	/*
		SendPacket()
			- WSASend!�� ���⿡���� ������ �� �ֽ��ϴ�.

		!0. �ܼ��� WSA Send�� �ִ� �Լ��Դϴ�. �����ʹ� �غ����ּ���.
		!1. wsaBuf�� len, wsaBuf�� buf�� �������ּž��մϴ�.
	*/
	void SendPacket(SocketInfo* pClient, SendMemoryUnit* pSendMemoryUnit)
	{
		// ���� �����Ϳ� ���� ó���� ������ �ٷ� �ٽ� ���� �غ�.
		DWORD flag{};

		ZeroMemory(&pSendMemoryUnit->overlapped, sizeof(pSendMemoryUnit->overlapped));

		ERROR_HANDLING::errorRecvOrSendArr[
			static_cast<bool>(
				1 + WSASend(pClient->sock, &pSendMemoryUnit->wsaBuf, 1, NULL, 0, &pSendMemoryUnit->overlapped, NULL)
				)
		]();
	}

	/*
		RecvPacket()
			- WSA Recv�� ���⼭�� �����մϴ�.

		!0. SocketInfo�� �ִ� wsaBuf -> buf �� ���ú긦 �մϴ�. 
		!1. len�� ������ ���� ����մϴ�. MAX_SIZE_OF_RECV_BUFFER!
	*/
	void RecvPacket(SocketInfo* pClient)
	{
		// ���� �����Ϳ� ���� ó���� ������ �ٷ� �ٽ� ���� �غ�.
		DWORD flag{};

		ZeroMemory(&pClient->overlapped, sizeof(pClient->overlapped));

		ERROR_HANDLING::errorRecvOrSendArr[
			static_cast<bool>(
				1 + WSARecv(pClient->sock, &pClient->wsaBuf, 1, NULL, &flag /* NULL*/, &pClient->overlapped, NULL)
				)
		]();
	}

	/*
		LogOutProcess()
			- �����ϴ�~~~
	*/
	void LogOutProcess(SocketInfo* pClient)
	{
		SOCKADDR_IN clientAddr;

		int addrLength = sizeof(clientAddr);

		getpeername(pClient->sock, (SOCKADDR *)&clientAddr, &addrLength);
		std::cout << " [GOODBYE] Ŭ���̾�Ʈ (" << inet_ntoa(clientAddr.sin_addr) << ") �� �����߽��ϴ�. \n";
		if (pClient->clientContIndex != -1) pClient->pScene->ExitClient(pClient->clientContIndex);

		closesocket(pClient->sock);
		delete pClient;
	}
}

namespace BIT_CONVERTER
{
	_NODISCARD BYTE MakeSendPacket(const BYTE inPacketType) noexcept { return inPacketType | SEND_BYTE; }

	_NODISCARD bool GetRecvOrSend(const char inChar) noexcept { return (inChar >> 7) & (0x01); }

	/*
		MakeByteFromLeftAndRightByte
			- 0x10���� ���� ����Ʈ �ΰ��� ���ڷ� �޾�(Left, Right) ���� 4����Ʈ�� Left, ���� 4�� ��Ʈ�� Right�� ��ƹ�ȯ�մϴ�.

		!0. ���ڷ� ������ �� ����Ʈ ���, 0x10���� ���� ���� ���;��մϴ�.
			- ���ڷ� ������ Left ����Ʈ�� 0x0f���� ū ���� ���� ���, �����÷ο�Ǿ� ���������� ���� ��Ⱥ�� �� ����.
			- ���ڷ� ������ Right ����Ʈ�� 0x0f���� ū ���� ���� ���, LeftByte�� | ���꿡�� ���������� ���� ��ȯ�� �� ����.
	*/
	_NODISCARD BYTE MakeByteFromLeftAndRightByte(const BYTE inLeftByte, const BYTE inRightByte) noexcept
	{
		return (inLeftByte << 4) | (inRightByte);
	}

	/*
		GetLeft4Bit
			- HIWORD(?)�� �����ϰ� �����մϴ�. �ϳ��� ����Ʈ�� �޾Ƽ� ����(����) 4���� ��Ʈ�� ����Ʈ�� ��ȯ�ؼ� ��ȯ�մϴ�.
	*/
	_NODISCARD BYTE GetLeft4Bit(const BYTE inByte) noexcept
	{
		return (inByte >> 4) & (0x0f);
	}

	/*
		GetRight4Bit
			- LOWORD(?)�� �����ϰ� �����մϴ�. �ϳ��� ����Ʈ�� �޾Ƽ� ����(����) 4���� ��Ʈ�� ����Ʈ�� ��ȯ�ؼ� ��ȯ�մϴ�.
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
			- ������ �ɰ��� ������ �߻��� ���, �޼��� �ڽ��� Ȱ���� ������ ����ϰ�, ������ �����մϴ�.
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
			- ������ ������ �߻��� ���, ���� �α׸� ����մϴ�.
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

		printf(" [%s]  %s", msg, (LPTSTR)&lpMsgBuf);
		LocalFree(lpMsgBuf);
	};

	/*
		HandleRecvOrSendError
			- Send Or Recv ���� �� ��µǴ� �Լ���, ������ ����մϴ�.
	*/
	void HandleRecvOrSendError()
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			ERROR_DISPLAY(("RecvOrSend()"));
		}
	}
}