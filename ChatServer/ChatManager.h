#pragma once

#include "InHeaderDefine.hh"

struct SocketInfo;
struct ZoneContUnit;

// 해당 ChatManager는 분리될 예정입니다.
struct ChatUnit
{
	static constexpr BYTE chatUnitCount{ 100 };

public:
	char message[GLOBAL_DEFINE::MAX_CHAT_MASSAGE_SIZE];
	
	/*
		client to server : wchar to char.
		server to client : char - char

		message[0] = Length;				//Fixed
		message[1] = type;					//Fixed
		message[2] = nickNameLength;
		message[3] ~ message[3 + nickNameLength * 2] = Nickname;
		message[3 + nickNameLength * 2 + 1] ~ message[Length] = ChatMessage;
	*/
};

class ChatManager
{
public:
	void ChatProcess(SocketInfo*, ZoneContUnit*);

public:
	ChatManager();
	~ChatManager();

public:
	Concurrency::concurrent_queue<ChatUnit*> sendedMessageCont;
};