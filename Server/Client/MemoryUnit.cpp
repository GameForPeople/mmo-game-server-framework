#include "stdafx.h"
#include "ClientDefine.h"
#include "MemoryUnit.h"

MemoryUnit::MemoryUnit(const bool InIsRecv) :
	overlapped(),
	wsaBuf(),
	dataBuf(nullptr),
	isRecv(InIsRecv)
{
	if (isRecv) {
		dataBuf = new char[GLOBAL_DEFINE::MAX_SIZE_OF_RECV];
		wsaBuf.len = GLOBAL_DEFINE::MAX_SIZE_OF_RECV;
	}
	else {
		dataBuf = new char[GLOBAL_DEFINE::MAX_SIZE_OF_SEND];
		wsaBuf.len = GLOBAL_DEFINE::MAX_SIZE_OF_SEND;
	}

	wsaBuf.buf = dataBuf;
}

MemoryUnit::~MemoryUnit()
{
	delete[] dataBuf;
}