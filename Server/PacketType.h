#pragma once
#include "pch.h"

enum class PACKET_TYPE : BYTE
{
	MOVE /* = 0*/,
	PacketTypeCount
};

constexpr int GetPacketTypeCount() noexcept 
{
	return static_cast<int>(PACKET_TYPE::PacketTypeCount);
}