#include "pch.h"

#include "ServerDefine.h"

namespace GLOBAL_UTIL
{
	namespace BIT_CONVERTER
	{
		_NODISCARD BYTE MakeSendPacket(const BYTE inPacketType) noexcept { return inPacketType | SEND_BYTE; }

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
				- 서버에 심각한 오류가 발생할 경우, 메세지 박스를 활용해 에러를 출력하고, 서버를 종류합니다.
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

			printf(" [%s]  %s", msg, (LPTSTR)&lpMsgBuf);
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
}