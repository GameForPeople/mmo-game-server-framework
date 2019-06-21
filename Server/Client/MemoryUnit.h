#pragma once

struct MemoryUnit
{
	OVERLAPPED overlapped;
	WSABUF wsaBuf;

	const bool isRecv;	// true == recv, false == send

	char* dataBuf;

public:
	MemoryUnit(const bool InIsRecv = false);
	~MemoryUnit();

	//MemoryUnit(const MemoryUnit& other);
	//MemoryUnit(MemoryUnit&& other) noexcept;
	//MemoryUnit& operator=(MemoryUnit&& other) noexcept;
};