/*
	kpu-warp-winapi-framework		Copyright ⓒ https://github.com/KPU-WARP

	#0. 해당 프로젝트는 한국산업기술대 게임공학과 학술 소모임 WARP의 오픈소스 WinAPI Framework입니다.
	#1. 관련 세부 정보는 깃헙 저장소 리드미에서 확인하실 수 있습니다.
		- https://github.com/KPU-WARP/winapi-framework

	#2. 기타 문의는, KoreaGameMaker@gmail.com으로 부탁드립니다. 감사합니다 :)
*/

#include "stdafx.h"

namespace GLOBAL_UTIL
{
	namespace BIT_CONVERTER
	{
		// 클라는 그대로 사용, 서버는 추후 인라인하기.

		/*
			GetLeft4Bit
				- HIWORD와 유사하게 동작합니다. 하나의 바이트를 받아서 상위(좌측) 4개의 비트를 바이트로 변환해서 반환합니다.
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

			std::cout << "[ERROR : " << msg << "] " << (LPTSTR)&lpMsgBuf;
			LocalFree(lpMsgBuf);
		};
	}
}
