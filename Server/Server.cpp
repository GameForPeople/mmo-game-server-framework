#include "pch.h"
#include "GameServer.h"

int main(int argc, char * argv[])
{
	GameServer::MakeInstance()->Run();

	GameServer::DestroyInstance();
}
