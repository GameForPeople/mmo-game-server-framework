#include "stdafx.h"

#include "ClientDefine.h"

namespace ERROR_HANDLING
{
	/*
		ERROR_QUIT
			- 서버에 심각한 오류가 발생할 경우, 메세지 박스를 활용해 에러를 출력하고, 서버를 종료합니다.
	*/
	_NORETURN void ERROR_QUIT(const WCHAR* msg)
	{
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			WSAGetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)& lpMsgBuf,
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
	/*_DEPRECATED*/ void ERROR_DISPLAY(const CHAR* msg)
	{
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			WSAGetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)& lpMsgBuf,
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