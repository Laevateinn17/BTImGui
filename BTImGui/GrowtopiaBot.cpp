#include "GrowtopiaBot.h"
//103.181.183.154
using namespace std;
GrowtopiaBot::GrowtopiaBot(const std::string& gid, const std::string& pw)
	:m_gid(gid), m_password(pw), m_peer(nullptr), m_host(nullptr)//, tBreak(std::chrono::milliseconds(1000))
{
	m_localBot.m_state = StateFlags::MOVE_FACE_RIGHT;
}

GrowtopiaBot::~GrowtopiaBot()
{
	try
	{
		if (m_peer)
		{
			this->Disconnect();		
			enet_peer_disconnect(this->m_peer, 0);
			enet_host_destroy(m_host);
		}
		m_running.store(false);
		m_execute.store(false);
		if (m_thread.joinable()) m_thread.join();
		if (m_executeThread.joinable()) m_executeThread.join();
		std::cout << "ss";
	}catch(...){}
}

int GrowtopiaBot::GetStatus() const
{
	return m_status;
}
Vector2<float> GrowtopiaBot::GetPos() const
{
	return m_localBot.m_pos;
}

Inventory& GrowtopiaBot::GetInventory()
{
	return m_inventory;
}
bool GrowtopiaBot::Disconnect()
{
	if (m_status < 0)
	{
		return false;
	}
	this->SendString(3, "action|quit");
	std::cout << "sent action|quit\n";
	m_status = 1;
	return true;
}
bool GrowtopiaBot::Connect(std::string ip, int port)
{
	try
	{

		if (ip.empty() && !port)
		{
			http::Request request{ "http://103.181.183.154" };
			const auto response = request.send("POST", "", std::vector<std::pair<std::string, std::string>>(), std::chrono::milliseconds(2000));
			rtvar var1 = rtvar::parse({ response.body.begin(), response.body.end() });
			//std::cout << var1.serialize();
			ip = var1.get("server");
			ip[ip.size()] = 0;
			port = var1.get_int("port");
			if (port == -1 || ip.empty())
			{
#ifndef NDEBUG
				std::cout << "Failed fetching server address\n";
#endif // !NDEBUG
				return false;
			}
		}
		//if (m_running.load()) m_running.store(false);
		ENetAddress addr;
		addr.port = port;
		enet_address_set_host_ip(&addr, ip.c_str());
		if (m_host) enet_host_destroy(m_host);
		this->m_host = enet_host_create(nullptr, 10, 0, 0, 0);
		this->m_host->usingNewPacket = true;
		this->m_host->checksum = enet_crc32;
		enet_host_compress_with_range_coder(this->m_host);
		this->m_peer = enet_host_connect(this->m_host, &addr, 10, 0);
		if (this->m_peer)
		{
			//enet_peer_timeout(this->m_peer, 5, 0, 0);
			enet_host_flush(this->m_host);
			//m_running.store(true);

			return true;
		}
		else
		{
#ifndef NDEBUG
			std::cout << ("ENet connection failed\n");
#endif
			return false;
		}
	}
	catch (std::exception& exception)
	{
#ifndef NDEBUG
		std::cout << "HTTP Request failed: " << exception.what();
#endif
		return false;
	}
	catch (...)
	{
		return false;
	}
}

void GrowtopiaBot::Poll()
{
	try
	{
		while (m_running.load())
		{
			ServiceLoop();
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}
	catch (...)
	{
		std::cout << "Caught exception in enet loop\n";
	}
}
void GrowtopiaBot::Start()
{
	m_thread = std::thread(&GrowtopiaBot::Poll, this);
}

void GrowtopiaBot::ServiceLoop()
{
	ENetEvent evt;
	//std::cout << "SERVICE LOOP\n";
	try
	{
		if (enet_host_service(m_host, &evt, 0) > 0)
		//{
		//	this->m_status = 0;
		//	m_meta = "";
		//	//enet_peer_disconnect(m_peer, 0);
		//	if (m_reconnect) Connect();
		//}
		//else
		{
			switch (evt.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
			{
#ifndef NDEBUG
				std::cout << this->m_gid << ": Connected\n";
#endif
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
			{
				this->m_status = 0;
				m_meta = "";
				if (m_reconnect)
				{
					std::cout << "---------RECONNECTING--------\n";
					std::this_thread::sleep_for(std::chrono::seconds(30));
					Connect();
				}
				break;
			}
			case ENET_EVENT_TYPE_RECEIVE:
			{
				int type = BTUtils::GetPacketType(evt.packet);
#ifndef NDEBUG
				//std::cout << "Type: " << type << '\n';
#endif
				switch (type)
				{
				case SERVER_HELLO:
				{
					this->OnLoginRequest();
					this->m_status = 1;
#ifndef NDEBUG
					std::cout << m_gid << " is Connected to Growtopia server\n";
#endif
					break;
				}
				case TEXT_PACKET:
				{
#ifndef NDEBUG
					std::cout <<  TextPacket(evt.packet) << '\n';
#else
					TextPacket(evt.packet);
#endif
					break;
				}
				case GAME_MESSAGE:
				{
#ifndef NDEBUG
					std::cout << "GAME MESSAGE" << GameMessage(evt.packet) << '\n';
#else
					GameMessage(evt.packet);
#endif
					break;
				}
				case GAME_PACKET:
				{
					ProcessGameUpdatePacket(evt.packet);
					break;
				}
				case TRACKING_PACKET:
				{
					ProcessTrackPacket(evt.packet);
					break;
				}
				default: 
					break;
				}
				break;
			}
			}
			//if (evt.packet) enet_packet_destroy(evt.packet);
			//enet_host_flush(m_host);
		}
	}
	catch (std::exception& exception)
	{
		std::cout << exception.what();
	}
	catch (...)
	{
		std::cout << "UNKNOWN EXCEPTION\n";
	}
}
void GrowtopiaBot::ProcessGameUpdatePacket(ENetPacket* packet)
{
	auto* gamePacket = BTUtils::GetStruct(packet);
	if (gamePacket)
#ifndef NDEBUG
		//std::cout << "Game packet type " << +gamePacket->type << '\n';
#endif
	switch (gamePacket->type)
	{
	case PACKET_STATE:
	{
		//OnPlayerState(packet);
		break;
	}
	case PACKET_CALL_FUNCTION:
	{
		OnPacketCallFunction(gamePacket);
		break;
	}
	case PACKET_SEND_TILE_TREE_STATE:
	{
		OnTreeState(gamePacket);
		break;
	}
	case PACKET_SEND_MAP_DATA:
	{
		OnMapData(gamePacket);
		break;
	}
	case PACKET_SEND_INVENTORY_STATE:
	{
		OnInventoryState(gamePacket);
		break;
	}
	case PACKET_MODIFY_ITEM_INVENTORY:
	{		
		OnModifyInventory(gamePacket);
		break;
	}
	case PACKET_APP_INTEGRITY_FAIL:
	{
		//gamePacket->type = 155;
		break;
	}
	case PACKET_ITEM_ACTIVATE_OBJECT_REQUEST:
	{
		break;
	}
	case PACKET_ITEM_ACTIVATE_REQUEST:
	{
		break;
	}
	case PACKET_ITEM_CHANGE_OBJECT:
	{
		OnObjectChange(gamePacket);
		break;
	}
	case PACKET_TILE_CHANGE_REQUEST:
	{
		OnTileChange(gamePacket);
		break;
	}
	case PACKET_SEND_ITEM_DATABASE_DATA:
	{
		//onSendItemsDat(gamePacket);
		break;
	}
	case PACKET_TILE_APPLY_DAMAGE:
	{
		break;
	}
	case PACKET_PING_REQUEST:
	{
		OnPingRequest(gamePacket);
		break;
	}
	case PACKET_NPC:
	{
		break;
	}
	default:
	{
		std::cout << "Unknown Tank packet type: " << +gamePacket->type << '\n';
		break;
	}
	}
}
void GrowtopiaBot::OnLoginRequest()
{
	auto HashStr = [&](std::string str) -> UINT { //refactored to perfection
		UINT buf = 0x55555555;
		for (BYTE& n : std::vector<BYTE>(str.begin(), str.end()))
			buf = (buf >> 27) + (buf << 5) + n;
		return buf;
	};
	std::mt19937 rng;
	std::uniform_int_distribution<int> distribution(INT_MIN, INT_MAX);
	m_mac = BTUtils::GenerateMac();
	rtvar loginVar;

	if (m_meta.empty())
	{
		auto hash_str = m_mac + "RT";
		http::Request request{ "http://103.181.183.154" };
		const auto response = request.send("POST");
		rtvar var1 = rtvar::parse({ response.body.begin(), response.body.end() });
		m_meta = var1.get("meta");
	}
	if (!m_password.empty())
	{
		loginVar.append("tankIDName|" + m_gid);
		loginVar.append("tankIDPass|" + m_password);
		loginVar.append("requestedName|SmileZero");
	}
	else
	{
		m_isGuest = true;
		loginVar.append("requestedName|" + m_gid);
	}
	loginVar.append("f|1");
	loginVar.append("protocol|175"); //sus
	loginVar.append("game_version|4.07");
	loginVar.append("fz|" + std::to_string(distribution(rng)));
	loginVar.append("lmode|1");
	loginVar.append("cbits|1024");
	loginVar.append("player_age|26");
	loginVar.append("GDPR|2");
	loginVar.append("category|_-5100");
	loginVar.append("totalPlaytime|0");
	loginVar.append("hash2|2028463651");// +std::to_string(HashStr(hash_str.c_str()))); //sus
	loginVar.append("meta|" + m_meta);
	loginVar.append("fhash|-716928004");
	loginVar.append("rid|" + BTUtils::RandomString(32, "0123456789ABCDEF"));
	//loginVar.append("gid|" + BTUtils::RandomString(32, "0123456789ABCDEF"));
	loginVar.append("platformID|0,1,1");
	loginVar.append("deviceVersion|0");
	loginVar.append("country|us");
	loginVar.append("hash|-559113089");//+ std::to_string(distribution(rng)));
	loginVar.append("mac|" + m_mac);
	if (this->m_user)
	{
		loginVar.append("user|" + std::to_string(m_user));
		loginVar.append("token|" + std::to_string(m_token));
		loginVar.append("UUIDToken|" + m_uuID);
		loginVar.append("doorID|" + m_doorID);
	}
	loginVar.append("wk|" + BTUtils::RandomString(32, "0123456789ABCDEF"));
	loginVar.append("zf|-1434588773");
	//loginVar.set("platformID", "4");
	//loginVar.remove("fz");
	std::cout << loginVar.serialize() << '\n';
	this->SendString(2, loginVar.serialize());

}

void GrowtopiaBot::SendUpdatePacket(int type, GameUpdatePacket* gamePacket)
{
	ENetPacket* packet = enet_packet_create(0, sizeof(GameUpdatePacket) + sizeof(type), ENET_PACKET_FLAG_RELIABLE);
	memcpy(packet->data, &type, sizeof(type));
	memcpy(packet->data + 4, gamePacket, sizeof(GameUpdatePacket));
	enet_peer_send(this->m_peer, 0, packet);
	//enet_host_flush(this->m_host);

}
void GrowtopiaBot::SendString(int type, std::string str) const
{
	if (str.size()) {
		ENetPacket* packet = enet_packet_create(0, str.size() + 5, ENET_PACKET_FLAG_RELIABLE);
		GameTextPacket* gamePacket = reinterpret_cast<GameTextPacket*>(packet->data);
		gamePacket->m_type = type;
		memcpy(&gamePacket->m_data, str.data(), str.size());
		memset(&gamePacket->m_data + str.size(), 0, 1);
		enet_peer_send(this->m_peer, 0, packet);
		//enet_host_flush(this->m_host);
	}
}

void GrowtopiaBot::Trash(int id, int count)
{
	if (m_inventory.inventory.find(id) == m_inventory.inventory.end()) return;
	this->SendString(2, "action|trash\n|itemID|" + std::to_string(id));
	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	this->SendString(2, "action|dialog_return\ndialog_name|trash_item\nitemID|" + std::to_string(id) + "|\ncount|" + std::to_string(count));
}

void GrowtopiaBot::OnTreeState(GameUpdatePacket* packet)
{
	if (packet->item == -1)
	{
		m_world.tiles.at(HashCoord(packet->int_x, packet->int_y)).header.fg = 0;
		//if (packet->net_id == m_localBot.m_netID)
			//tBreak.Unlock();
	}

}

std::string GrowtopiaBot::TextPacket(ENetPacket* packet)
{
	std::string ss;
	ss.resize(packet->dataLength - 2);
	memcpy(ss.data(), packet->data + 1, packet->dataLength - 2);
	return ss;
}

std::string GrowtopiaBot::GameMessage(ENetPacket* packet)
{
	std::string ss;
	ss.resize(packet->dataLength - 2);
	memcpy(ss.data(), packet->data + 1, packet->dataLength - 2);
	if (ss.find("action|log\n") != std::string::npos)
	{
		if (ss.find("If you don't have one") != std::string::npos)
		{
			m_loginMessage = "Incorrect GrowID or Password!";
			m_running.store(false);
			m_status = -2;
		}
		else if (ss.find("temporary ban caused by") != std::string::npos)
		{
			m_running.store(false);
			m_status = -1;
			if (ss.find(m_gid) != std::string::npos)
			{
				m_status = -3;
			}
		}
		else if (ss.find("this account is currently banned") != std::string::npos)
		{
			m_running.store(false);
			m_status = -3;
		}
		else if (ss.find("has been suspended") != std::string::npos)
		{
			m_running.store(false);
			m_status = -3;
		}
		else if (ss.find("account for guest") != std::string::npos)
		{
			m_running.store(false);
			m_loginMessage = "Couldn't create guest account!";
			m_status = -2;
		}
		else if (ss.find("is now available for your device") != std::string::npos)
		{
			m_loginMessage = "Update required!";
			m_status = -2;
		}
		else if (ss.find("3 chars") != std::string::npos)
		{
			m_loginMessage = "GrowID needs to be at least 3 chars!";
			m_status = -2;
		}
		else if (ss.find("Too many people logging"))
		{
			std::this_thread::sleep_for(std::chrono::seconds(2));
			Connect();
		}
	}
	if (ss.find("action|logon_fail") != std::string::npos)
	{
		this->m_user = 0;
		this->m_token = 0;
		this->m_ip = "";
		this->m_port = 0;
		this->m_meta = "";
		this->Disconnect();
		//this->Connect();
		//this->Connect();
	}

	return ss;
}

const std::string& GrowtopiaBot::GetLoginMessage() const
{
	return this->m_loginMessage;
}

void GrowtopiaBot::OnObjectChange(GameUpdatePacket* packet)
{
	if (packet->net_id == -1)
	{
		Object object{};
		object.count = (int)packet->obj_alt_count;
		object.index = ++m_world.objectLastIndex;
		object.id = packet->int_data;
		object.x = packet->vec_x;
		object.y = packet->vec_y;
		m_world.objects.set(object.index, object);
		if (m_autoCollect) Collect(m_collectRange);

	}
	else if (packet->net_id == -3)
	{
		//if (m_world.objects.find(packet->item_net_id) == m_world.objects.end()) return;
		Object& obj = m_world.objects.get(packet->item_net_id);
		obj.id = packet->item_id;
		obj.count = packet->obj_alt_count;
		obj.x = packet->vec_x;
		obj.y = packet->vec_y;
	}
	else 
	{
		if (packet->net_id == m_localBot.m_netID)
		{
			Object& obj = m_world.objects.get(packet->item_id);
			if (obj.id == 112)
			{
				m_gems += obj.GetCount();
			}
			else
			{
				if (m_inventory.inventory.find(obj.id) != m_inventory.inventory.end())
				{
					m_inventory.inventory.at(obj.id).count = obj.count + m_inventory.GetCount(obj.id) > 200 ? 200 : m_inventory.inventory.at(obj.id).count + obj.count;
				}
				else
				{
					InventoryItem item{};
					item.id = obj.id;
					item.count = obj.count;
					m_inventory.inventory[obj.id] = item;
				}
			}
		}
		//inventoryTask.Unlock();
		m_world.objects.erase(packet->int_data);
	}
}

void GrowtopiaBot::Collect(int r)
{
	try
	{
		int range = r * 32;
		for (auto& object : m_world.objects.c_)
		{
			if (std::abs((this->m_localBot.m_pos.x) - (object.second.x)) < range && std::abs((this->m_localBot.m_pos.y) - (object.second.y)) < range)
			{
				GameUpdatePacket packet{};
				packet.type = PACKET_ITEM_ACTIVATE_OBJECT_REQUEST;
				packet.int_data = object.second.index;
				packet.vec_x = object.second.x;//m_localBot.m_pos.x;
				packet.vec_y = object.second.y;//m_localBot.m_pos.y;
				//packet.int_x = object.second.x + object.second.y + 4;
				this->SendUpdatePacket(4, &packet);
			}
		}
	}
	catch (std::exception& exception)
	{
		std::cout << "Exception in GrowtopiaBot::Collect() : " << exception.what();
	}
	catch (...)
	{
	}
}
void GrowtopiaBot::OnTileChange(GameUpdatePacket* packet)
{
	Tile& tile = m_world.tiles[HashCoord(packet->int_x, packet->int_y)];
	if (packet->int_data == 0x12)
	{
		if (tile.header.fg) tile.header.fg = 0;
		else tile.header.bg = 0;
		//if (packet->net_id == m_localBot.m_netID)
			//tBreak.Unlock();
	}
	else
	{
		switch (itemsDat.itemMap[packet->int_data]->actionType)
		{
		case 18:
			tile.header.bg = packet->int_data;
			break;
		case 19:
			tile.header.fg = packet->int_data;
			tile.header.growTime = itemsDat.itemMap[tile.header.fg]->growTime;
			tile.header.elapsedTime = 0;
			tile.header.snapshotTime = std::time(nullptr);
			break;
		default:
			tile.header.fg = packet->int_data;
			break;
		}
		if (packet->net_id == m_localBot.m_netID)
		{
			if (--m_inventory.inventory[packet->int_data].count == 0) m_inventory.inventory.erase(packet->int_data);
		}
		//tPlace.Unlock();
	}
}

void GrowtopiaBot::OnModifyInventory(GameUpdatePacket* packet)
{
	InventoryItem& item = m_inventory.inventory[packet->int_data];
	if (packet->count_1 > item.count)
	{
		item.count -= packet->count_1;
	}
	else
	{
		m_inventory.inventory.erase(item.id);
	}
}

void GrowtopiaBot::OnPacketCallFunction(GameUpdatePacket* packet)
{
	if (packet->type)
	{
		uint8_t* extended = BTUtils::GetExtended(packet);
		variantlist_t varlist;
		varlist.serialize_from_mem(extended);
#ifndef NDEBUG
		std::cout << varlist.print() << '\n';
#endif
		std::string funcName{ varlist.get(0).get_string() };
		if (funcName == "OnSendToServer")
		{
			this->m_token = varlist.get(2).get_uint32() != -1 ? varlist.get(2).get_uint32() : this->m_token;
			this->m_user = varlist.get(3).get_uint32();
			std::string cpy = varlist.get(4).get_string();
			std::vector<std::string> vec;
			BTUtils::Split(cpy, '|', vec);
			this->m_doorID = vec[1].empty() ? "0" : vec[1];
			this->m_uuID = vec[2] == "-1" ? this->m_uuID : vec[2];
			this->m_port = varlist.get(1).get_int32();
			this->m_ip = vec[0];
			this->Connect(m_ip, m_port);
		}
		else if (funcName.find("OnEmoticonDataChanged") != std::string::npos)
		{
		}
		else if (funcName.find("OnSuperMainStartAcceptLogon") != std::string::npos)
		{
			this->m_status = 2;
			cv.notify_one();
			this->SendString(2, "action|enter_game\n");
		}
		else if (funcName.find("SetHasGrowID") != std::string::npos)
		{
			this->m_gid = varlist.get(2).get_string();
		}
		else if (funcName.find("OnRequestWorldSelectMenu") != std::string::npos)
		{
			this->m_targetWorld = "";
			this->m_world.name = "EXIT";
		}
		else if (funcName.find("OnSpawn") != std::string::npos)
		{
			std::string str = varlist.get(1).get_string();
			rtvar var = rtvar::parse(str);
			auto name = var.find("name");
			auto netid = var.find("netID");
			auto onlineid = var.find("onlineID");
			if (name && netid)
			{
				Player player;
				player.m_name = name->m_value.substr(2, name->m_value.size() - 4);
				player.m_netID = var.get_int("netID");
				player.m_userID = var.get_int("userID");
				player.m_country = var.get("country");
				auto pos = var.find("posXY");
				player.m_pos.x = (float)atoi(pos->m_values[0].c_str());
				player.m_pos.y = (float)atoi(pos->m_values[1].c_str());
				if (var.get("mstate") == "1" || var.get("smstate") == "1") player.m_isMod = true;
				if (var.get("type") == "local")
				{
					this->m_localBot.m_name = player.m_name;
					this->m_localBot.m_netID = player.m_netID;
					this->m_localBot.m_pos = player.m_pos;
				}
				m_world.players[player.m_netID] = player;
				if (player.m_isMod)
				{
					this->Warp("EXIT");
					this->Disconnect();
					this->m_status = -5;
					m_execute.store(false);
				}
			}
		}
		else if (funcName.find("OnRemove") != std::string::npos)
		{
			std::string str = varlist.get(1).get_string();
			rtvar var = rtvar::parse(str);
			m_world.players.erase(var.get_int("netID"));
		}
		else if (funcName.find("OnSetPos") != std::string::npos)
		{
			m_localBot.m_pos.x = varlist.get(1).get_vector2().m_x;
			m_localBot.m_pos.y = varlist.get(1).get_vector2().m_y;
		}
		else if (funcName.find("onShowCaptcha") != std::string::npos)
		{
			auto menu = varlist[1].get_string();
			std::vector<std::string> g{};
			BTUtils::Split(menu, '|', g);
			std::string captchaid = g[1];
			BTUtils::Replace(captchaid, "0098/captcha/generated/", "");
			BTUtils::Replace(captchaid, "PuzzleWithMissingPiece.rttex", "");
			captchaid = captchaid.substr(0, captchaid.size() - 1);
			std::string SecretCode = "c2ee5206-6c5e-404c-9e37-1c960a2cc184daafaac02f462b0";
			auto x = PuzzleSolver(SecretCode);
			x.GetAnswer(captchaid);
			if (x.Solved) {
				this->SendString(2, "action|dialog_return\ndialog_name|puzzle_captcha_submit\ncaptcha_answer|" + x.LatestAnswer + "|CaptchaID|" + g[4]);
			}
		}
		else if (funcName.find("OnConsoleMessage") != std::string::npos)
		{
			std::string param = varlist.get(2).get_string();
			if (param.find("is now level") != std::string::npos)
			{
				if (param.find(BTUtils::ToLower(m_gid)) != std::string::npos)
				{
					std::cout << "LEVEL UP YAHOOOOOOOOOOO\n";
				}
			}

		}
	}
}

void GrowtopiaBot::Equip(int id)
{
	if (m_inventory.inventory.find(id) == m_inventory.inventory.end()) return;
	GameUpdatePacket packet{};
	packet.type = PACKET_ITEM_ACTIVATE_REQUEST;
	packet.item_id = id;
	this->SendUpdatePacket(4, &packet);
}
void GrowtopiaBot::Drop(int id, int count)
{
	if (m_inventory.inventory.find(id) == m_inventory.inventory.end()) return;
	if (count > m_inventory.inventory.at(id).count) return;
	if (count == 0)
	{
		count = m_inventory.inventory.at(id).count;
	}
	SendString(2, "action|drop\n|itemID|" + std::to_string(id));
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	SendString(2, "action|dialog_return\ndialog_name|drop_item\nitemID|" + std::to_string(id) + "|\ncount|" + std::to_string(count));

}

void GrowtopiaBot::SetCollectRange(int range)
{
	m_collectRange = range;
}

void GrowtopiaBot::Place(int x, int y, int id)
{
	//if (std::abs(x) > 2 || std::abs(y) > 2) return;
	if (m_inventory.inventory.find(id) == m_inventory.inventory.end()) return;
	GameUpdatePacket packet{};
	packet.type = PACKET_TILE_CHANGE_REQUEST;
	packet.vec_x = m_localBot.m_pos.x;
	packet.vec_y = m_localBot.m_pos.y;
	packet.item_id = id;
	packet.int_x = BTUtils::tile(m_localBot.m_pos.x) + x;
	packet.int_y = BTUtils::tile(m_localBot.m_pos.y) + y;
	//packet.flags = StateFlags::TILE_CHANGE_RIGHT;
	this->SendUpdatePacket(4, &packet);
	/*packet.type = PACKET_STATE;
	packet.flags = StateFlags::PLACE_FACE_RIGHT;*/
	
	//tPlace.Lock();
}

void GrowtopiaBot::Punch(int x, int y)
{
	GameUpdatePacket packet{};
	x = (this->GetPos().x / 32) + x;
	y = (this->GetPos().y / 32) + y;
	packet.type = PACKET_TILE_CHANGE_REQUEST;
	packet.int_data = 18;
	//packet.flags = StateFlags::TILE_CHANGE_RIGHT;
	packet.vec_x = this->GetPos().x;
	packet.vec_y = this->GetPos().y;
	packet.int_x = x;
	packet.int_y = y;
	this->SendUpdatePacket(4, &packet);
	/*packet.type = PACKET_STATE;
	packet.flags = StateFlags::PUNCH_FACE_RIGHT;
	packet.int_data = 0;
	this->SendUpdatePacket(4, &packet);*/
	//this->UpdateLocalState();
	//packet.type = PACKET_TILE_APPLY_DAMAGE;
	//packet.net_id = m_localBot.m_netID;
	//if (m_inventory.GetCount(98))
	//{
	//	packet.int_data = 8;
	//	packet.item = 98;
	//}
	//else packet.int_data = 6;
	//packet.int_x = x;
	//packet.int_y = y;
	////packet.flags = StateFlags::TILE_CHANGE_RIGHT;
	//this->SendUpdatePacket(4, &packet);
	//tBreak.Lock();b
}

void GrowtopiaBot::UpdateLocalState()
{
	GameUpdatePacket packet{};
	packet.type = PACKET_STATE;
	packet.flags = m_localBot.m_state;
	packet.pos_x = m_localBot.m_pos.x;
	packet.pos_y = m_localBot.m_pos.y;
	packet.int_x = -1;
	packet.int_y = -1;
	//packet.net_id = m_localBot.m_netID;
	this->SendUpdatePacket(GAME_PACKET, &packet);
	if (m_autoCollect) Collect(m_collectRange);
}
void GrowtopiaBot::ProcessTrackPacket(ENetPacket* packet)
{
	std::string ss;
	ss.resize(packet->dataLength - 2);
	memcpy(ss.data(), packet->data + 1, packet->dataLength - 2);
	//std::cout << ss;
	rtvar var(rtvar::parse(ss));
	if (var.validate_int("Gems_balance"))
	{
		m_gems = var.get_int("Gems_balance");
	}
	if (var.validate_int("Level"))
	{
		m_level = var.get_int("Level");
	}
}
int GrowtopiaBot::GetGems() const
{
	return m_gems;
}
uint8_t GrowtopiaBot::GetLevel() const
{
	return m_level;
}

void GrowtopiaBot::Warp(const std::string& world)
{
	this->SendString(3, "action|join_request\nname|" + world + "\ninvitedWorld|0");
}
void GrowtopiaBot::MoveRight()
{
	if (m_status != 2)
	{
		return;
	}
	int x = (int)(m_localBot.m_pos.x / 32) + 1;
	int y = (int)(m_localBot.m_pos.y / 32);
	uint32_t hash = HashCoord(x, y);
	int cType = m_world.tiles.at(hash).collisionType;
	if (cType == 1 || cType == 9) return;
	this->m_localBot.m_pos.x += 32;
	//this->m_localBot.m_state = ;
	this->UpdateLocalState();
}


void GrowtopiaBot::MoveLeft()
{
	if (m_status != 2)
	{
		return;
	}
	int x = (int)(m_localBot.m_pos.x / 32) - 1;
	int y = (int)(m_localBot.m_pos.y / 32);
	uint32_t hash = HashCoord(x, y);
	int cType = m_world.tiles.at(hash).collisionType;
	if (cType == 1 || cType == 9) return;
	this->m_localBot.m_pos.x -= 32;
	//this->m_localBot.m_state = StateFlags::MOVE_FACE_RIGHT;
	this->UpdateLocalState();
}
void GrowtopiaBot::MoveUp()
{
	if (m_status != 2)
	{
		return;
	}
	int x = (int)m_localBot.m_pos.x / 32;
	int y = (int)(m_localBot.m_pos.y / 32) - 1;
	uint32_t hash = HashCoord(x, y);
	int cType = m_world.tiles.at(hash).collisionType;
	if (cType == 1 || cType == 9) return;
	this->m_localBot.m_pos.y -= 32;
	//this->m_localBot.m_state = StateFlags::MOVE_FACE_RIGHT;
	this->UpdateLocalState();
}
void GrowtopiaBot::MoveDown()
{
	if (m_status != 2)
	{
		return;
	}
	int x = (int)m_localBot.m_pos.x / 32;
	int y = (int)(m_localBot.m_pos.y / 32) + 1;
	int hash = HashCoord(x, y);
	int cType = m_world.tiles.at(hash).collisionType;
	if (cType == 1 || cType == 9) return;
	this->m_localBot.m_pos.y += 32;
	//this->m_localBot.m_state = StateFlags::MOVE_FACE_RIGHT;
	this->UpdateLocalState();
}

void GrowtopiaBot::ReadJSON(std::string path, int index)
{
	std::ifstream file(path.c_str());
	using json = nlohmann::json;
	json data = json::parse(file);
	m_executeThread = std::thread(&GrowtopiaBot::Rotasi, this, data);
}
void GrowtopiaBot::Rotasi(nlohmann::json data)
{
	printf("rotasi bos\n");
	try
	{
		using namespace std::chrono_literals;
		auto arr = data["worlds"];
		int farmable = data["farmable"].get<int>();
		int delay = data["delay"].get<int>();
		std::string storage = data["seedStorage"];
		std::string  packStorage = data["packStorage"];
		int price = data["price"];
		std::string pack = data["pack"];
		std::vector<int> dropArray = data["drop"];
		std::vector<int> trashArray = data["trash"];
		//std::vector<int> trashEx = data["trashEx"];
		auto StoreSeed = [&] {
			//m_autoCollect = false;
			while (true)
			{
				if (storage.find(m_world.name) != std::string::npos)
				{
					if (m_world.GetTile(m_localBot.m_pos.x / 32, m_localBot.m_pos.y / 32).header.fg != 6)
					{
						std::this_thread::sleep_for(2s);
						break;
					}
					else
					{
						this->Warp(storage);
						std::this_thread::sleep_for(4s);
					}
				}
				this->Warp(storage);
				std::this_thread::sleep_for(5s);
			}
			for (auto& tile : m_world.tiles)
			{
				if (m_status != 2) break;
				if (tile.second.header.fg == 880)
				{
					int seedCount = 0;
					int i = 0;
					for (auto& object : m_world.objects.c_)
					{
						//if (object.second.id != farmable + 1) continue;
						if (BTUtils::tileRound(object.second.x) == tile.second.header.pos.x && BTUtils::tile(object.second.y) == tile.second.header.pos.y)
							seedCount += object.second.count;
						++i;
					}
					if (seedCount + m_inventory.GetCount(farmable + 1) > 4000) continue;
					if (FindPath(tile.second.header.pos.x - 1, tile.second.header.pos.y))
					{
						std::this_thread::sleep_for(1s);
						this->Drop(farmable + 1);
						std::this_thread::sleep_for(3s);
						if (m_inventory.GetCount(farmable + 1) == 0) break;
					}
				}
			}
			//m_autoCollect = true;
		};
		auto DumpPacks = [&]() {
			//m_autoCollect = false;
			while (true)
			{
				if (packStorage.find(m_world.name) != std::string::npos)
				{
					if (m_world.GetTile(m_localBot.m_pos.x / 32, m_localBot.m_pos.y / 32).header.fg != 6)
					{
						std::this_thread::sleep_for(2s);
						break;
					}
					else
					{
						this->Warp(packStorage);
						std::this_thread::sleep_for(4s);
					}
				}
				this->Warp(packStorage);
				std::this_thread::sleep_for(5s);
			}
			for (auto& tile : m_world.tiles)
			{
				bool dropped = true;
				if (tile.second.header.fg == 880)
				{
					if (FindPath(tile.second.header.pos.x - 1, tile.second.header.pos.y))
					{
						std::this_thread::sleep_for(1s);
						for (auto& i : dropArray)
						{
							if (m_inventory.GetCount(i) > 0)
							{
								this->Drop(i);
								std::this_thread::sleep_for(2s);
								if (m_inventory.GetCount(i) > 0)
								{
									dropped = false;
									break;
								}
							}
						}
					}
					if (dropped) break;
				}
			}
			//m_autoCollect = true;
		};
		bool skipWorld = false;
		for (int i = 0; i < arr.size(); ++i)
		{
			std::string dest = arr[i].get<std::string>();
			if (dest.find(m_world.name) != std::string::npos)
				skipWorld = true;
		}
		while (m_execute)
		{
			for (int i = 0; i < arr.size(); ++i)
			{
				Timer timer;
				if (arr[i].get<std::string>().find(m_world.name) != std::string::npos) skipWorld = false;
				if (skipWorld) continue;
				std::string dest = arr[i].get<std::string>();
				auto WarpFarm = [&]() {
					while (true)
					{
						if (m_status != 2) break;
						if (dest.find(m_world.name) != std::string::npos)
						{
							if (m_world.GetTile(m_localBot.m_pos.x / 32, m_localBot.m_pos.y / 32).header.fg != 6)
							{
								std::this_thread::sleep_for(4s);
								break;
							}
							else
							{
								this->Warp(dest);
								std::this_thread::sleep_for(4s);
							}
						}
						this->Warp(dest);
						std::this_thread::sleep_for(5s);
					}
				};
				for (auto& i : trashArray)
				{
					if (m_inventory.GetCount(i) > 0)
					{
						this->Trash(i, m_inventory.GetCount(i));
						std::this_thread::sleep_for(3s);
					}
				}
				if (!pack.empty() && !packStorage.empty())
				{
					if (GetGems() >= price * 5)
					{
						while (GetGems() >= price)
						{
							if (m_status != 2) break;
							for (auto& i : dropArray)
							{
								if (m_inventory.GetCount(i) == 200) DumpPacks();
							}
							this->SendString(2, "action|buy\nitem|" + pack);
							std::this_thread::sleep_for(1s);
						}
					}
					for (auto& i : dropArray)
					{
						if (m_inventory.GetCount(i))
						{
							DumpPacks();
							break;
						}
					}
				}
				if (m_inventory.GetCount(farmable + 1) > 100)
				{
					StoreSeed();
				}
				WarpFarm();
				bool farming = true;
				//m_autoCollect = true;
				while (farming)
				{
					farming = false;
					for (auto& tile : m_world.tiles)
					{
						if (m_status != 2) break;
						if (tile.second.header.fg == farmable + 1 && tile.second.IsReady())
						{
							farming = true;
							if (m_inventory.GetCount(farmable) < 150 || (tile.second.header.pos.x == 0	|| tile.second.header.pos.x == 99))
							{
								if (FindPath(tile.second.header.pos.x, tile.second.header.pos.y))
								{
									std::this_thread::sleep_for(100ms);
									Punch(0, 0);
									std::this_thread::sleep_for(150ms);
									Collect(3);
									if (m_inventory.GetCount(farmable + 1) > 0)
									{
										Place(0, 0, farmable + 1);
										std::this_thread::sleep_for(100ms);
									}
									if (m_inventory.GetCount(farmable + 1) >= 190)
									{
										break;
									}
								}
							}
							else
							{
								if (tile.second.header.pos.x != 0 && tile.second.header.pos.x != 99)
								{
									if (!FindPath(tile.second.header.pos.x, tile.second.header.pos.y)) break;
									std::this_thread::sleep_for(200ms);
									while (m_inventory.GetCount(farmable) > 0)
									{
										if (m_status != 2) break;
										Vector2<int> pos = { BTUtils::tile(m_localBot.m_pos.x), BTUtils::tile(m_localBot.m_pos.y) };
										Collect(3);
										if (m_inventory.GetCount(farmable + 1) >= 180) break;
										if (m_world.GetTile(pos.x, pos.y).header.fg == 0)
										{
											Place(0, 0, farmable);
											std::this_thread::sleep_for(std::chrono::milliseconds(delay - 50 < 130 ? 130 : delay - 50));
										}
										while (m_world.GetTile(pos.x, pos.y).header.fg != 0)
										{
											if (m_status != 2) break;
											Punch(0, 0);
											std::this_thread::sleep_for(std::chrono::milliseconds(delay));
										}
									}
									break;
								}
							}
						}
						else if ((tile.second.header.fg == 0 || tile.second.header.fg == farmable) && m_world.GetTile(tile.second.header.pos.x, tile.second.header.pos.y + 1).collisionType == 1)
						{
							if (m_inventory.GetCount(farmable + 1) == 0) continue;
							if (FindPath(tile.second.header.pos.x, tile.second.header.pos.y))
							{
								std::this_thread::sleep_for(std::chrono::milliseconds(150));
								while (tile.second.header.fg == farmable)
								{
									Punch(0, 0);
									std::this_thread::sleep_for(std::chrono::milliseconds(delay));
								}
								Place(0, 0, farmable + 1);
								std::this_thread::sleep_for(std::chrono::milliseconds(100));
							}
						}
					}
					if (m_inventory.GetCount(farmable + 1) >= 150 && BTUtils::tile(m_localBot.m_pos.x) < 49)
					{
						StoreSeed();
						std::this_thread::sleep_for(2s);
						WarpFarm();
					}
					if (!m_execute) return;
					if (m_status != 2)
					{
						std::mutex mtx;
						std::unique_lock<std::mutex> ul(mtx);
						cv.wait(ul, [&]() -> bool { return m_status != 0; });
						std::this_thread::sleep_for(15s);
						WarpFarm();
						std::this_thread::sleep_for(3s);
					}
				}
			}
		}
	}
	catch(std::exception& exception)
	{
		std::cout << "Executor thread exception: " << exception.what();
	}
	catch (...)
	{
		std::cout << "error in executor thread\n";
	}
}

bool GrowtopiaBot::FindPath(int x, int y)
{
	//2.8
	if (x < 0 || x > 99 || y < 0 || y > 53 || m_world.GetTile(x, y).collisionType != 0) return false;
	Vector2<int> start = { BTUtils::tile(GetPos().x), BTUtils::tile(GetPos().y) };
	Vector2<int> dest{ x, y };
	int steps = 0;
	std::queue<Vector2<int>> queue{};
	//std::stack<Vector2<int>> stack{};
	std::unordered_map<int, bool> visited;
	std::unordered_map<int, int> parent;
	queue.push(start);
	//stack.push(start);
	visited[HashCoord(start.x, start.y)] = true;
	std::vector<Vector2<int>> path;
	while (!queue.empty())
	{
		Vector2<int> tile = queue.front();
		queue.pop();
		if (tile == dest) break;
		for (auto& i : m_world.GetAdjacentTile(tile))
		{
			int hash = HashCoord((i.header.pos.x), (i.header.pos.y));
			if (visited[hash] || i.collisionType != 0) continue;
			visited[hash] = true;
			parent[hash] = HashCoord(tile.x, tile.y);
			queue.push(i.header.pos);
		}

	}
	Vector2<int> currPath = m_world.GetTile(x, y).header.pos;
	while (parent.find(HashCoord(currPath.x, currPath.y)) != parent.end())
	{
		path.push_back(currPath);
		currPath = { parent[HashCoord(currPath.x, currPath.y)] % 100, parent[HashCoord(currPath.x, currPath.y)] / 100 };
	}
	//timer.~Timer();
	int c = 1;
	for (std::vector<Vector2<int>>::reverse_iterator i = path.rbegin();
		i != path.rend(); ++i)
	{
		if (c % 7 == 0 || *i == dest)
		{
			m_localBot.m_pos.x = i->x * 32.f;
			m_localBot.m_pos.y = i->y * 32.f;
			this->UpdateLocalState();
			std::this_thread::sleep_for(std::chrono::milliseconds(70));
		}
		++c;
	}
	return true;
}

void GrowtopiaBot::OnMapData(GameUpdatePacket* packet)
{
	auto extended = BTUtils::GetExtended(packet);
	int version = *extended;
	extended += 6;
	uint16_t nameLength = *(uint16_t*)(extended);
	extended += 2;
	m_world.name.resize(nameLength);
	memcpy(m_world.name.data(), extended, nameLength);
	extended += (uint64_t)(nameLength);
	m_world.width = *(uint32_t*)(extended);
	m_world.height = *(uint32_t*)(extended += 4);
	m_world.tileCount = *(uint32_t*)(extended += 4);
	extended += 4;
	m_world.invalid_tile.header.fg = -1;
	this->m_world.Serialize(extended);
}

void GrowtopiaBot::OnInventoryState(GameUpdatePacket* packet)
{
	uint8_t* extended = BTUtils::GetExtended(packet);
	++extended; //idk what the first byte means yet
	m_inventory.slotCount = *(uint32_t*)extended;
	extended += 4;
	m_inventory.usedSlot = *(uint16_t*)extended;
	extended += 2;
	for (int i = 0; i < m_inventory.usedSlot; ++i)
	{
		uint16_t itemID = *(uint16_t*)extended;
		extended += 2;
		uint8_t count = *extended;
		++extended;
		uint8_t flag = *extended;
		++extended;
		InventoryItem invItem{ itemID, count, flag };
		m_inventory.inventory[itemID] = (invItem);
	}
	std::cout << m_inventory.slotCount << " slot inventory : \n";
}

void GrowtopiaBot::OnPingRequest(GameUpdatePacket* gamePacket)
{
	std::unique_ptr<GameUpdatePacket> packet(new GameUpdatePacket());
	packet->type = PACKET_PING_REPLY;
	packet->vec_x = 64.0;
	packet->vec_y = 64.0;
	packet->hash = gamePacket->hash + 5000;
	this->SendUpdatePacket(4, packet.get());
	std::cout << "SENT PING RPELY\n";

}

const World& GrowtopiaBot::GetWorld() const
{
	return this->m_world;
}

bool GrowtopiaBot::IsGuest() const
{
	return m_isGuest;
}

void GrowtopiaBot::Interact()
{
	GameUpdatePacket packet{};
	packet.type = PACKET_TILE_ACTIVATE_REQUEST;
	packet.int_data = 18;
	packet.vec_x = GetPos().x;
	packet.vec_y = GetPos().y;
	packet.int_x = BTUtils::tile(GetPos().x);
	packet.int_y = BTUtils::tile(GetPos().y);
	this->SendUpdatePacket(4, &packet);
}

void GrowtopiaBot::Say(const std::string& str) const
{
	this->SendString(2, "action|input\n|text|" + str);
}
