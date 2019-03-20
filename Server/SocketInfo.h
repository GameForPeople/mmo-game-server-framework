#pragma once

#include "pch.h"
#include "UserData.h"

struct SocketInfo
{
	static constexpr int BUFFER_MAX_SIZE = 13;

	// Protocol[1Byte] + MyPos[1Byte] + connectedClientCount[1Byte] + (connectedClientCount * Pos)Byte..!
	// 1 + 1 + 1 + 10..! 13.

public:
	SocketInfo() noexcept
		: overlapped()
		, wsabuf()
		, socket()
		, buf()
		, userData(new UserData(0, 0)/*std::make_unique<UserData>(0, 0)*/)
	{}

	~SocketInfo()
	{
		delete userData;
	}

public:
	OVERLAPPED overlapped;	// OVERLAPPED ±¸Á¶Ã¼
	WSABUF wsabuf;
	SOCKET socket;
	
	char buf[BUFFER_MAX_SIZE];

	UserData* userData;
	//std::unique_ptr<UserData> userData;
};