#pragma once
#include "GrowtopiaBot.h"
#include <string>
#include <memory>
#include <vector>
#include <cassert>
#include "Utilities.h"
class Bot
{
private:
	std::string m_growID;
	std::string m_password;
	std::shared_ptr<GrowtopiaBot>m_bot = nullptr;
public:
	bool m_initializeInventoryTab = true;
	~Bot();
	Bot() = default;
	Bot(const Bot&) = default;
	Bot(const std::string& gid, const std::string& pw) noexcept;
	std::string GetGID() const;
	std::shared_ptr<GrowtopiaBot> GetGrowtopiaBot();
};

class BotArray
{
	std::vector<std::shared_ptr<Bot>> m_bots;
public:
	BotArray() = default;
	bool Add(const char* gid, const char* password);
	bool AddGuest(const char* gid);
	bool Remove(std::string& gid);
	bool Remove(int index);
	bool HasGuest() const ;
	int Find(std::string& gid);
	int Find(const char* gid);
	std::shared_ptr<Bot> operator[](int index);
	size_t size();
};