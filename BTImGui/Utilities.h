#pragma once
#include <string>
#include <utility>
#include <random>
#include <ctime>
#include "Struct.h"
#include "enet/include/enet.h"
namespace BTUtils
{
	uint32_t GetPacketType(ENetPacket* packet);
	void Split(std::string& str, char delimiter, std::vector<std::string>& arr);
	GameUpdatePacket* GetStruct(ENetPacket* packet);
	uint8_t* GetExtended(GameUpdatePacket* packet);
	std::string GenerateMac();
	std::string RandomString(int len, std::string charset);
	bool strcmpi(const std::string& str1, const std::string& str2);
	std::string ToLower(const std::string& str);
	int tileRound(float pos);
	int tile(float pos);
	bool Replace(std::string& str, const std::string& from, const std::string& to);
}