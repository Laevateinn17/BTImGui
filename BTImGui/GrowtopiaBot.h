#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <iostream>
#include "GamePacket.h"
#include "nlohmann/json.hpp"
#include <vector>
#include <functional>
#include <unordered_map>
#include <queue>
#include <stack>
#include <cmath>
#include <mutex>
#include <random>
#include <ctime>
#include <future>
#include "Struct.h"
#include "world.h"
#include "Utilities.h"
#include "HTTPRequest.hpp"
#include "proton/rtparam.hpp"
#include "enet/include/enet.h"
#include "TaskAsync.h"
class GrowtopiaBot
{

private:
	bool m_isGuest = false;
	std::string m_gid = "";
	std::string m_password = "";
	std::string m_mac = "";
	std::string m_meta = "";
	uint32_t m_token = 0;
	uint32_t m_user = 0;
	std::string m_doorID = "";
	std::string m_uuID = "";
	std::string m_targetWorld;
	ENetPeer* m_peer = nullptr;
	ENetHost* m_host = nullptr;
	std::string m_ip = "";
	int m_port = 0;
	LocalPlayer m_localBot;
	uint8_t m_level = 0;
	int m_gems = 0;
	std::thread m_thread;	
	std::thread m_executeThread;
	//TaskAsync tBreak, tPlace;	
	std::condition_variable cv;
	bool m_reconnect = true;
	std::mutex m_mutex;
	std::atomic_bool m_running = true;
	std::atomic_bool m_execute = true;
	//TaskAsync inventoryTask;
	int m_collectRange = 3;
	int m_status = 0;
	std::string m_loginMessage = "";
	World m_world;
	Inventory m_inventory;
	void ServiceLoop();
	void Poll();
	std::string TextPacket(ENetPacket* packet);
	std::string GameMessage(ENetPacket* packet);
	void ProcessGameUpdatePacket(ENetPacket* packet);
	void OnPlayerState(ENetPacket* packet);
	void OnPacketCallFunction(GameUpdatePacket* packet);
	void OnTileChange(GameUpdatePacket* packet);
	void OnTreeState(GameUpdatePacket* packet);
	void OnMapData(GameUpdatePacket* packet);
	void OnSendItemsDat(GameUpdatePacket* packet);
	void OnInventoryState(GameUpdatePacket* packet);
	void OnModifyInventory(GameUpdatePacket* packet);
	void OnObjectChange(GameUpdatePacket* packet);
	void OnPingRequest(GameUpdatePacket* gamePacket);
	void OnLoginRequest();
	void ProcessTrackPacket(ENetPacket* packet);
	void UpdateLocalState();
	void Reconnect();

public:
	bool m_autoCollect = false;
	void MoveRight();
	void MoveLeft();	
	void MoveUp();
	void MoveDown();
	bool FindPath(int x, int y);
	void Punch(int x, int y);
	void Place(int x, int y, int id);
	void SetCollectRange(int range);
	void Collect(int range = 1);
	void Drop(int id, int count = 0);
	bool Reachable(int x, int y);
	void Trash(int id, int count);
	void Equip(int id);
	void Warp(const std::string& world);
	void Interact();
	void SendString(int type, std::string str) const;
	void Say(const std::string& str) const;
	void SendUpdatePacket(int type, GameUpdatePacket* packet);
	void ReadJSON(std::string path, int index);
	void Start();
	void Rotasi(nlohmann::json);
	~GrowtopiaBot();
	GrowtopiaBot() = default;
	GrowtopiaBot(const GrowtopiaBot&) = default;
	GrowtopiaBot(const std::string& gid, const std::string& pw);
	bool Disconnect();
	bool Connect(std::string ip = "", int port = 0);
	int GetStatus() const;
	int GetGems() const;
	const std::string& GetLoginMessage() const;
	const World& GetWorld() const;
	uint8_t GetLevel() const;
	Vector2<float> GetPos() const;
	Inventory& GetInventory();
	bool IsGuest() const;

};
