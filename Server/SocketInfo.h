#pragma once

class UserData;
class Scene;

/*
	SocketInfo
		- 소켓정보구조체 입니다.

	!0. 다음 멤버 변수 선언 순서는 절대로 보장되야합니다.
		- 가장 먼저 overlapped 구조체, WSABUF, char*의 순서는 항상 보장되야 합니다.
*/
struct SocketInfo
{
public:
	SocketInfo() /*noexcept*/;
	~SocketInfo();

public:
	OVERLAPPED overlapped;	// OVERLAPPED 구조체
	WSABUF wsaBuf;
	char* recvBuf;
	int loadedSize;
	char *loadedBuf; 

	SOCKET sock;
	UserData* userData;
	//std::unique_ptr<UserData> userData;

	int clientContIndex;
	Scene* pScene;
};