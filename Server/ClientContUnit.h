#pragma once

#include "InHeaderDefine.hh"

struct SocketInfo;

struct ZoneContUnit
{
	std::vector<_ClientNode> clientCont;
	std::shared_mutex wrlock;
};

struct SectorContUnit
{
	std::list<_ClientKeyType> clientCont;
	std::shared_mutex wrlock;
};