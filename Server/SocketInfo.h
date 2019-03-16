#pragma once

#include "pch.h"
#include "UserData.h"

struct SocketInfo
{
	static constexpr int BUFFER_MAX_SIZE = 2;

public:
	SocketInfo() noexcept
		: overlapped()
		, wsabuf()
		, sock()
		, buf()
		, userData(std::make_unique<UserData>(0, 0))
	{
	}
	~SocketInfo() = default;

public:
	OVERLAPPED overlapped;	// OVERLAPPED ±¸Á¶Ã¼
	WSABUF wsabuf;
	SOCKET sock;

	char buf[BUFFER_MAX_SIZE];

	std::unique_ptr<UserData> userData;
	//std::shared_ptr<UserData> userData;
};