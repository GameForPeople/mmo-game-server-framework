/*
	kpu-warp-winapi-framework		Copyright ⓒ https://github.com/KPU-WARP

	#0. 해당 프로젝트는 한국산업기술대 게임공학과 학술 소모임 WARP의 오픈소스 WinAPI Framework입니다.
	#1. 관련 세부 정보는 깃헙 저장소 리드미에서 확인하실 수 있습니다.
		- https://github.com/KPU-WARP/winapi-framework

	#2. 기타 문의는, KoreaGameMaker@gmail.com으로 부탁드립니다. 감사합니다 :)
*/

// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 특정 포함 파일이 들어 있는
// 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#define _WINSOCK_DEPRECATED_NO_WARNINGS	// inet_addr

#include <windows.h>
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "wininet.lib")
#include <WinSock2.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <iostream>

#include <atlimage.h>	// CImage

#include <string>
#include <string_view> 

#include <functional>

#include <bitset>	// debug

#include <thread>	//thread

#include <mutex>

#include <array>

#include <cassert>

#include "Define.h"
#include "GameFramework.h"

#define		_NORETURN			[[noreturn]]
#define		_NODISCARD			[[nodiscard]]
#define		_DEPRECATED			[[deprecated]]
#define		_MAYBE_UNUSED		[[maybe_unused]]
#define		_FALLTHROUGH		[[fallthrough]]

namespace GLOBAL_UTIL {
	namespace BIT_CONVERTER {
		_NODISCARD BYTE GetLeft4Bit(const BYTE inByte) noexcept;
		_NODISCARD BYTE GetRight4Bit(const BYTE inByte) noexcept;
	}

	namespace ERROR_HANDLING {
		_NORETURN void ERROR_QUIT(const WCHAR *msg);
		/*_DEPRECATED*/ void ERROR_DISPLAY(const CHAR *msg);
	}
}