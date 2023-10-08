#include "Utilities.h"

namespace BTUtils
{
	int tileRound(float pos)
	{
		return pos / 32 + 0.5;
	}
	uint32_t HashBytes(std::string str)
	{
		UINT buf = 0x55555555;
		for (BYTE& n : std::vector<BYTE>(str.begin(), str.end()))
			buf = (buf >> 27) + (buf << 5) + n;
		return buf;
	};
	uint32_t GetPacketType(ENetPacket* packet)
	{
		return (packet->dataLength > 3 ? *packet->data : 0);
	}
	void Split(std::string& str, char delimiter, std::vector<std::string>& arr)
	{
		size_t start = 0;
		for (size_t i = 0; i < str.size(); ++i)
		{
			if (str[i] == delimiter)
			{
				arr.push_back(str.substr(start, i - start));
				start = i + 1;
			}
		}
		arr.push_back(str.substr(start, str.size() - start));
	}
	GameUpdatePacket* GetStruct(ENetPacket* packet)
	{

		if (packet->dataLength >= sizeof(GameUpdatePacket)) {
			auto* gameUpdatePacket = reinterpret_cast<GameUpdatePacket*>(packet->data + 4);
			if (!(gameUpdatePacket->flags & 8))
				return gameUpdatePacket;

			if (packet->dataLength < gameUpdatePacket->data_size + sizeof(GameUpdatePacket)) {
				return nullptr;
			}
			return gameUpdatePacket;
		}
		return nullptr;
	}

	uint8_t* GetExtended(GameUpdatePacket* packet)
	{
		if (!(packet->flags & 8))
			return nullptr;

		struct ExtendedPacket {
			uint8_t pad[sizeof(GameUpdatePacket)];
			uint32_t data;
		};

		return reinterpret_cast<uint8_t*>(&reinterpret_cast<ExtendedPacket*>(packet)->data);
	}

	std::string GenerateMac()
	{
		std::string newmac = "";
		char nums[] = "0123456789ABCDEF";
		std::srand((uint32_t)std::time(nullptr));
		for (int i = 1; i <= 12; ++i)
		{
			newmac += nums[std::rand() % (sizeof(nums) - 1)];
			if (i % 2 == 0 && i != 12)
			{
				newmac += ":";
			}
		}
		return newmac;
	}

	std::string RandomString(int len, std::string charset)
	{
		std::string str = "";
		for (int i = 0; i < len; ++i)
		{
			str += charset[rand() % (charset.size() - 1)];
		}
		return str;
	}
	bool strcmpi(const std::string& str1, const std::string& str2)
	{
		if (str1.size() != str2.size() || (str1.empty() && str2.empty())) return false;
		for (int i = 0; i < str1.size(); ++i)
		{
			char c1 = str1[i] < 'a' ? str1[i] + 32 : str1[i];
			char c2 = str2[i] < 'a' ? str2[i] + 32 : str2[i];
			if (c1 != c2) return false;
		}
		return true;
	}	
	
	int tile(float pos)
	{
		return pos / 32;
	}
	std::string ToLower(const std::string& str)
	{
		std::string s = "";
		for (auto& i : str)
		{
			if (i >= 'A' && i <= 'Z') s += (i - 0x20);
		}
		return s;
	}
	bool Replace(std::string& str, const std::string& from, const std::string& to) {
		size_t start_pos = str.find(from);
		if (start_pos == std::string::npos)
			return false;
		str.replace(start_pos, from.length(), to);
		return true;
	}
}