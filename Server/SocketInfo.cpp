#include "pch.h"
#include "Define.h"
#include "ServerDefine.h"

#include "UserData.h"
#include "SocketInfo.h"
#include "Scene.h"

SocketInfo::SocketInfo() /*noexcept*/
	: overlapped()
	, wsaBuf()
	, sock()
	, recvBuf()
	, loadedSize()
	, loadedBuf()
	, userData(new UserData(0, 0)/*std::make_unique<UserData>(0, 0)*/)
	, clientContIndex(-1)
	, pScene(nullptr)
{
	recvBuf = new char[GLOBAL_DEFINE::MAX_SIZE_OF_RECV];
	loadedBuf = new char[GLOBAL_DEFINE::MAX_SIZE_OF_RECV_PACKET];
}

SocketInfo::~SocketInfo()
{
	delete[] recvBuf;
	delete[] loadedBuf;

	delete userData;
}