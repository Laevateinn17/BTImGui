#include "Bot.h"


Bot::Bot(const std::string& gid, const std::string& pw) noexcept
	: m_growID(gid), m_password(pw), m_bot(new GrowtopiaBot(m_growID, m_password))
{
	
}

Bot::~Bot()
{
	std::cout << "Destructor called\n";
}

std::string Bot::GetGID() const
{
	return m_growID;
}

std::shared_ptr<GrowtopiaBot> Bot::GetGrowtopiaBot() 
{
	return m_bot;
}

bool BotArray::Add(const char* gid, const char* password)
{
	std::string growid = gid;
	int index = this->Find(growid);
	if (index != -1 && !m_bots[index]->GetGrowtopiaBot()->IsGuest()) return false;

	m_bots.push_back(std::make_shared<Bot>(std::move(Bot(growid, password))));
	return true;
}
bool BotArray::AddGuest(const char* gid)
{
	m_bots.push_back(std::make_shared<Bot>(std::move(Bot(gid, ""))));
	return true;
}

bool BotArray::Remove(std::string& gid)
{
	int index = this->Find(gid);
	if (index == -1) return false;

	m_bots.erase(m_bots.begin() +index);
	return true;
}

bool BotArray::Remove(int index)
{
	assert(index >= 0 && index < m_bots.size());
	m_bots.erase(m_bots.begin() + index);
	return true;
}

int BotArray::Find(std::string& gid)
{
	for (int i  = 0; i < m_bots.size(); ++i)
	{
		if (BTUtils::strcmpi(m_bots[i]->GetGID(), gid))
		{
			return i;
		}
	}
	return -1;
}

int BotArray::Find(const char* gid)
{
	for (int i = 0; i < m_bots.size(); ++i)
	{
		if (BTUtils::strcmpi(m_bots[i]->GetGID(), gid))
		{
			return i;
		}
	}
	return -1;
}
std::shared_ptr<Bot> BotArray::operator[](int index)
{
	assert(index >= 0 && index < m_bots.size());
	return m_bots[index];
}

size_t BotArray::size()
{
	return m_bots.size();
}

bool BotArray::HasGuest() const
{
	for (auto& i : m_bots)
	{
		if (i->GetGrowtopiaBot()->IsGuest())
			return true;
	}
	return false;
}