#include "pch.h"
#include "../Define.h"
#include "GameChatServer.h"

/*
	게임 서버 프레임워크 프로젝트
			- 한국산업기술대 게임공학과 13학번 원성연 (2013182027)
	
	https://github.com/GameForPeople/mmo-game-server-framework
*/

int main(int argc, char * argv[])
{
#ifdef UNICODE
	UNICODE_UTIL::SetLocaleToKorean();
#endif

	std::unique_ptr<GameChatServer> gameChatServer
		= std::make_unique<GameChatServer>
		(
			[/* void */]() noexcept(false) -> bool
			{

#ifdef _DEV_MODE_
				std::cout << " 주의!! _DEV_MODE_가 활성화 되어 있습니다. " << std::endl;
#endif
				return true;
			}()
		);

	gameChatServer->Run();
}
