#pragma once

struct MemoryUnit;

namespace ERROR_HANDLING {

	inline void NotError(void) {};
	void HandleRecvOrSendError(void);

	_NORETURN void ERROR_QUIT(const WCHAR* msg);
	/*_DEPRECATED*/ void ERROR_DISPLAY(const WCHAR* msg);
}