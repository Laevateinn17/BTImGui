#pragma once
#include <cstdint>
#include <memory>
#include <fstream>
#include <unordered_map>
#include <map>
#include <chrono>
#include <iostream>
#include <future>
#include <mutex>
#include "HTTPRequest.hpp"

//template <typename T, typename U>
//class Map : public std::map<T, U>
//{
//    std::mutex mtx;
//    std::future<void> checker;
//    std::condition_variable c_var;
//    std::atomic_bool flag = true;
//public:
//    void Lock()
//    {
//        flag.store(false);
//    }
//    void Unlock() {
//        flag.store(true);
//        c_var.notify_one();
//    }
//    void Add(T index, U item)
//    {
//        if (!flag.load())
//        {
//            std::unique_lock<std::mutex> lock(mtx);
//            c_var.wait(lock, [&]() -> bool { return flag.load(); });
//        }
//        this[index] = item;
//    }
//};
template<class Key, class Value>
class ThreadSafeMap
{
    std::mutex m_;

public:
    std::map<Key, Value> c_;
    Value& get(Key const& k) {
        std::unique_lock<decltype(m_)> lock(m_);
        return c_[k]; // Return a copy.
    }

    //Value get(Key const& k) {
    //    std::unique_lock<decltype(m_)> lock(m_);
    //    return c_[k]; // Return a copy.
    //}

    template<class Value2>
    void set(Key const& k, Value2&& v) {
        std::unique_lock<decltype(m_)> lock(m_);
        c_[k] = std::forward<Value2>(v);
    }
    void erase(Key const& k) {
        std::unique_lock<decltype(m_)> lock(m_);
        c_.erase(k);
    }
};

class PuzzleSolver {
public:
    bool Solved = false;
    std::string LatestAnswer = "";
    PuzzleSolver(std::string g) : SecretKey(g) {}
    void AddParams(std::string Key, std::string Value)
    {
        Params.append(Key + "=" + Value + "&");
    }

    std::string GetAnswer(std::string CaptchaUID)
    {
        AddParams("Action", "Solve");
        AddParams("Puzzle", CaptchaUID);
        AddParams("Secret", SecretKey);
        AddParams("Format", "txt");
        std::string FullUrl = API + Path + Params;
        http::Request request{ std::string{FullUrl}.c_str() };
        const auto response = request.send("GET");
        std::string captchaAnswer = std::string{ response.body.begin(), response.body.end() };
        Solved = captchaAnswer.find("Failed") == std::string::npos && captchaAnswer.length() > 6 && response.status.code == 202;

        return LatestAnswer = (captchaAnswer.length() > 6) ? captchaAnswer.erase(0, 7) : "Failed";

    }
private:
    std::string SecretKey;
    std::string API = "http://api.surferwallet.net/";
    std::string Path = "Captcha?";
    std::string Params = "";
};

struct ItemData {
    int itemID = 0;
    char editableType = 0;
    char itemCategory = 0;
    unsigned char actionType = 0;
    char hitSoundType = 0;
    std::string name = "";
    std::string texture = "";
    int textureHash = 0;
    char itemKind = 0;
    int val1 = 0;
    char textureX = 0;
    char textureY = 0;
    char spreadType = 0;
    char isStripeyWallpaper = 0;
    char collisionType = 0;
    uint8_t breakHits = 0;
    int dropChance = 0;
    char clothingType = 0;
    int16_t rarity = 0;
    unsigned char maxAmount = 0;
    std::string extraFile = "";
    int extraFileHash = 0;
    int audioVolume = 0;
    std::string petName = "";
    std::string petPrefix = "";
    std::string petSuffix = "";
    std::string petAbility = "";
    char seedBase = 0;
    char seedOverlay = 0;
    char treeBase = 0;
    char treeLeaves = 0;
    int seedColor = 0;
    int seedOverlayColor = 0;
    int growTime = 0;
    short val2 = 0;
    short isRayman = 0;
    std::string extraOptions = "";
    std::string texture2 = "";
    std::string extraOptions2 = "";
    std::string punchOptions = "";
};

struct ARGB
{
    uint8_t a, r, g, b;
};

class ItemContainer
{
public:
    std::unordered_map<int, std::shared_ptr<ItemData>> itemMap;
    bool LoadItemsData(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        int64_t size = file.tellg();
        if (size == -1)
        {
            return false;
        }
        file.seekg(0, file.beg);
        std::string secret = "PBG892FXX982ABC*";
        char* data = new char[size];
        file.read(data, size);
        int itemCount;
        int memPos = 0;
        int16_t itemsdatVersion = 0;
        memcpy(&itemsdatVersion, data + memPos, 2);
        memPos += 2;
        memcpy(&itemCount, data + memPos, 4);
        memPos += 4;
        for (int i = 0; i < itemCount; ++i)
        {
            std::shared_ptr<ItemData> item(new ItemData());
            {
                memcpy(&item->itemID, data + memPos, 4);
                memPos += 4;
            }
            {
                item->editableType = data[memPos];
                memPos += 1;
            }
            {
                item->itemCategory = data[memPos];
                memPos += 1;
            }
            {
                item->actionType = data[memPos];
                memPos += 1;
            }
            {
                item->hitSoundType = data[memPos];
                memPos += 1;
            }
            {
                int16_t strLen = *(int16_t*)&data[memPos];
                memPos += 2;
                for (int j = 0; j < strLen; j++)
                {
                    item->name += data[memPos] ^ (secret[((int64_t)j + item->itemID) % secret.length()]);
                    memPos++;
                }
            }
            {
                int16_t strLen = *(int16_t*)&data[memPos];
                memPos += 2;
                for (long j = 0; j < strLen; j++)
                {
                    item->texture += data[memPos];
                    memPos++;
                }
                memcpy(&item->textureHash, data + memPos, 4);
                memPos += 4;
                item->itemKind = memPos[data];
                memPos += 1;
                memcpy(&item->val1, data + memPos, 4);
                memPos += 4;
                item->textureX = data[memPos];
                memPos += 1;
                item->textureY = data[memPos];
                memPos += 1;
                item->spreadType = data[memPos];
                memPos += 1;
                item->isStripeyWallpaper = data[memPos];
                memPos += 1;
                item->collisionType = data[memPos];
                memPos += 1;
                item->breakHits = data[memPos] / 6;
                memPos += 1;
                memcpy(&item->dropChance, data + memPos, 4);
                memPos += 4;
                item->clothingType = data[memPos];
                memPos += 1;
                memcpy(&item->rarity, data + memPos, 2);
                memPos += 2;
                item->maxAmount = data[memPos];
                memPos += 1;
                {
                    int16_t strLen = *(int16_t*)&data[memPos];
                    memPos += 2;
                    for (int j = 0; j < strLen; j++)
                    {
                        item->extraFile += data[memPos];
                        memPos++;
                    }
                }
                memcpy(&item->extraFileHash, data + memPos, 4);
                memPos += 4;
                memcpy(&item->audioVolume, data + memPos, 4);
                memPos += 4;
                {
                    int16_t strLen = *(int16_t*)&data[memPos];
                    memPos += 2;
                    for (int j = 0; j < strLen; j++)
                    {
                        item->petName += data[memPos];
                        memPos++;
                    }
                }
                {
                    int16_t strLen = *(int16_t*)&data[memPos];
                    memPos += 2;
                    for (int j = 0; j < strLen; j++)
                    {
                        item->petPrefix += data[memPos];
                        memPos++;
                    }
                }
                {
                    int16_t strLen = *(int16_t*)&data[memPos];
                    memPos += 2;
                    for (int j = 0; j < strLen; j++)
                    {
                        item->petSuffix += data[memPos];
                        memPos++;
                    }
                }
                {
                    int16_t strLen = *(int16_t*)&data[memPos];
                    memPos += 2;
                    for (int j = 0; j < strLen; j++)
                    {
                        item->petAbility += data[memPos];
                        memPos++;
                    }
                }
                {
                    item->seedBase = data[memPos];
                    memPos += 1;
                }
                {
                    item->seedOverlay = data[memPos];
                    memPos += 1;
                }
                {
                    item->treeBase = data[memPos];
                    memPos += 1;
                }
                {
                    item->treeLeaves = data[memPos];
                    memPos += 1;
                }
                {
                    memcpy(&item->seedColor, data + memPos, 4);
                    memPos += 4;
                }
                {
                    memcpy(&item->seedOverlayColor, data + memPos, 4);
                    memPos += 4;
                }
                memPos += 4; // deleted ingredients
                {
                    memcpy(&item->growTime, data + memPos, 4);
                    memPos += 4;
                }
                memcpy(&item->val2, data + memPos, 2);
                memPos += 2;
                memcpy(&item->isRayman, data + memPos, 2);
                memPos += 2;
                {
                    int16_t strLen = *(int16_t*)&data[memPos];
                    memPos += 2;
                    for (int j = 0; j < strLen; j++)
                    {
                        item->extraOptions += data[memPos];
                        memPos++;
                    }
                }
                {
                    int16_t strLen = *(int16_t*)&data[memPos];
                    memPos += 2;
                    for (int j = 0; j < strLen; j++)
                    {
                        item->texture2 += data[memPos];
                        memPos++;
                    }
                }
                {
                    int16_t strLen = *(int16_t*)&data[memPos];
                    memPos += 2;
                    for (int j = 0; j < strLen; j++)
                    {
                        item->extraOptions2 += data[memPos];
                        memPos++;
                    }
                }
                memPos += 80;
                if (itemsdatVersion >= 11)
                {
                    {
                        int16_t strLen = *(int16_t*)&data[memPos];
                        memPos += 2;
                        for (int j = 0; j < strLen; j++)
                        {
                            item->punchOptions += data[memPos];
                            memPos++;
                        }
                    }
                }
                if (itemsdatVersion >= 12)
                {
                    memPos += 13;
                }
                if (itemsdatVersion >= 13)
                {
                    memPos += 4;
                }
                if (itemsdatVersion >= 14)
                    memPos += 4;
                itemMap[item->itemID] = item;
            }
        }
        file.close();
        return true;
    }
    bool hasTileExtra(int id)
    {
        int actionType = this->itemMap.find(id)->second->actionType;
        return
            actionType == 2 || // Door
            actionType == 3 || // Lock
            actionType == 10 || // Sign
            actionType == 13 || // Main Door
            actionType == 19 || // Seed
            actionType == 26 || // Portal
            actionType == 33 || // Mailbox
            actionType == 34 || // Bulletin Board
            actionType == 36 || // Roshambo Block n dice
            actionType == 38 || // Chemical Source
            actionType == 40 || // Achievement Block
            actionType == 43 || // Sungate
            actionType == 46 ||
            actionType == 47 ||
            actionType == 49 ||
            actionType == 50 ||
            actionType == 51 || // Bunny Egg
            actionType == 52 ||
            actionType == 53 ||
            actionType == 54 || // Xenonite
            actionType == 55 || // Phone Booth
            actionType == 56 || // Crystal
            id == 2246 || // Crystal
            actionType == 57 || // Crime In Progress
            actionType == 59 || // Spotlight
            actionType == 61 ||
            actionType == 62 ||
            actionType == 63 || // Fish Wall Port
            id == 3760 || // Data Bedrock
            actionType == 66 || // Forge
            actionType == 67 || // Giving Tree
            actionType == 71 || //steam organ
            actionType == 72 ||
            actionType == 73 || // Sewing Machine
            actionType == 74 ||
            actionType == 75 ||
            actionType == 76 || // Painting Easel
            actionType == 77 || //pet cage
            actionType == 78 || // Pet Trainer (WHY?!)
            actionType == 79 || //steam engine
            actionType == 80 || // Lock-Bot (Why?!)
            actionType == 81 ||
            actionType == 82 || //SSU
            actionType == 83 || // Display Shelf
            actionType == 84 ||
            actionType == 85 || // Challenge Timer
            actionType == 86 || // Challenge Start/End Flags
            actionType == 87 || // Fish Wall Mount
            actionType == 88 || // Portrait
            actionType == 89 ||
            actionType == 91 || // Fossil Prep Station
            actionType == 92 ||
            actionType == 93 || // Howler
            actionType == 97 || // Storage Box Xtreme / Untrade-a-box
            actionType == 98 ||
            actionType == 99 ||
            actionType == 100 || // Geiger Charger
            actionType == 101 ||
            actionType == 102 || //tomb robber
            actionType == 111 || // Magplant
            actionType == 113 || // CyBot
            actionType == 115 || // Lucky Token
            actionType == 116 || // GrowScan 9000 ???
            actionType == 126 ||//cloud storm
            actionType == 127 ||//temp plat 
            actionType == 130 ||
            actionType == 134 || //infinity weather
            actionType == 141 || // kranken's galactic block
            (id % 2 == 0 && id >= 5818 && id <= 5932) ||
            // ...
            false;
    }
};

struct MapData {
public:
    uint16_t tileFormatVersion;
    uint32_t unknown;
    uint16_t nameLength;
    std::string name;
    int width;
    int height;
    int tileCount;
};

template <typename T>
struct Vector2
{
    T x;
    T y;
    friend bool operator==(const Vector2<T>& operand, const Vector2<T>& operand2)
    {
        return operand.x == operand2.x && operand.y == operand2.y;
    }
};

    class Timer
{
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
public:
    Timer()
        :start(std::chrono::high_resolution_clock::now())
    {
    }
    ~Timer()
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto startTime = std::chrono::time_point_cast<std::chrono::minutes>(start).time_since_epoch().count();
        auto endTime = std::chrono::time_point_cast<std::chrono::minutes>(end).time_since_epoch().count();
        std::cout << "Process took " << (endTime - startTime) << "minutes\n";
    }
};

struct TileHeader
{
    uint16_t fg;
    uint16_t bg;
    uint16_t data;
    uint8_t flag1;
    uint8_t flag2;
    Vector2<int> pos;
    uint32_t elapsedTime;
    time_t snapshotTime, growTime;
};

class Tile
{
public:
    TileHeader header;
    int collisionType;
    bool IsReady() const
    {
        return (header.fg % 2 == 1 && std::time(nullptr) - header.snapshotTime + header.elapsedTime >= header.growTime);
    }
};

struct Object
{
    uint32_t index;
    uint16_t id;
    float x;
    float y;
    uint8_t count;
    uint8_t flag;
    int GetCount() const
    {
        return count;
    }

};

struct InventoryItem
{
    uint16_t id;
    uint8_t count;
    uint8_t equipped;
};

struct Inventory
{
    int usedSlot = 0;
    int slotCount = 0;
    std::unordered_map<int, InventoryItem> inventory;
    int GetCount(int id) const
    {
        return inventory.find(id) != inventory.end() ? inventory.at(id).count : 0;
    }
};

struct Objects
    {};

class Player
{
public:
    std::string m_name = "";
    std::string m_world = "";
    std::string m_country = "";
    Vector2<float> m_pos{};
    int m_netID = 0;
    int m_userID = 0;
    bool m_isMod = 0;
    bool m_invis = 0;
    int m_state = 0;
};


class LocalPlayer : public Player
{
private:
    std::vector<InventoryItem> m_inventory;
public:

};

enum ExtraType
{
    DOOR = 1,
    SIGN,
    LOCK,
    SEED,
    NONE,
    MAILBOX,
    BULLETIN_BOX,
    DICE,
    PROVIDER,
    ACHIEVEMENT_BLOCK = 0xA,
    HMON = 0xB,
    MANNEQUIN = 0xE,
    BUNNY_EGG = 0xF,
    GAME_GRAVE = 0x10,
    //GAME_GEN = 0x11,
    XENONITE = 0x12,
    TRANSMUTABOOTH = 0x13,
    CRYSTAL = 0x14,
    VILLAIN = 0x15,
    SPOTLIGHT = 0x16,
    DISPLAY_BLOCK = 0x17,
    VENDING = 0x18,
    FISH_PORT = 0x19,
    FORGE = 0x1B,
    GIVING_TREE = 0x1C,
    STEAM_ORGAN = 0x1E,
    SILKWORM = 0x1F,
    SEWING_MACHINE = 0x20,
    COUNTRY_FLAG = 0x21,
    LOBSTER_TRAP = 0x22,
    PAINTING_EASEL = 0x23,
    PET_CAGE = 0x24,
    PET_TRAINER = 0x25,
    STEAM_ENGINE = 0x26,
    LOCK_BOT = 0x27,
    WEATHER = 0x28,
    SSU = 0x29,
    DATA_BEDROCK = 0x2A,
    DISPLAY_SHELF = 0x2B,
    VIP_ENTRANCE = 0x2C,
    CHALLENGE_TIMER = 0x2D,
    FISH_MOUNT = 0x2F,
    PORTRAIT = 0x30,
    WEATHER_STUFF = 0x31,
    FOSSIL_PREP = 0x32,
    DNA_PROCESSOR = 0x33,
    HOWLER = 0x34,
    STORAGE_BOX = 0x36,
    HOME_OVEN = 0x37,
    AUDIO_RACK = 0x38,
    ADVENTURE = 0x3A,
    TOMB_ROBBER = 0x3B,
    GAUT = 0x3E,
    GUILD_FLAG = 0x41,
    GROWSCAN = 0x42,
    STORM_CLOUD = 0x48,
    DISAPPEARING_PLAT = 0x49,
    WEATHER_INFINITY = 0x4D,
    KRANKEN = 0x50,
    FRIENDS_ENTRANCE = 0x51,

};


enum Packet
{
    MESSAGE_UNKNOWN = 0,
    SERVER_HELLO,
    TEXT_PACKET,
    GAME_MESSAGE,
    GAME_PACKET,
    ERROR_PACKET,
    TRACKING_PACKET,
};


enum {
    PACKET_STATE = 0,
    PACKET_CALL_FUNCTION,
    PACKET_UPDATE_STATUS,
    PACKET_TILE_CHANGE_REQUEST,
    PACKET_SEND_MAP_DATA,
    PACKET_SEND_TILE_UPDATE_DATA, //? sent while accessing to world lock
    PACKET_SEND_TILE_UPDATE_DATA_MULTIPLE,
    PACKET_TILE_ACTIVATE_REQUEST,
    PACKET_TILE_APPLY_DAMAGE,
    PACKET_SEND_INVENTORY_STATE,
    PACKET_ITEM_ACTIVATE_REQUEST, //14?
    PACKET_ITEM_ACTIVATE_OBJECT_REQUEST,
    PACKET_SEND_TILE_TREE_STATE,
    PACKET_MODIFY_ITEM_INVENTORY,
    PACKET_ITEM_CHANGE_OBJECT,
    PACKET_SEND_LOCK,
    PACKET_SEND_ITEM_DATABASE_DATA,
    PACKET_SEND_PARTICLE_EFFECT,
    PACKET_SET_ICON_STATE,
    PACKET_ITEM_EFFECT,
    PACKET_SET_CHARACTER_STATE,
    PACKET_PING_REPLY,
    PACKET_PING_REQUEST,
    PACKET_GOT_PUNCHED,
    PACKET_APP_CHECK_RESPONSE,
    PACKET_APP_INTEGRITY_FAIL,
    PACKET_DISCONNECT,
    PACKET_BATTLE_JOIN,
    PACKET_BATTLE_EVEN,
    PACKET_USE_DOOR,
    PACKET_SEND_PARENTAL,
    PACKET_GONE_FISHIN,
    PACKET_STEAM,
    PACKET_PET_BATTLE,
    PACKET_NPC,
    PACKET_SPECIAL,
    PACKET_SEND_PARTICLE_EFFECT_V2,
    GAME_ACTIVE_ARROW_TO_ITEM,
    GAME_SELECT_TILE_INDEX
};
enum StateFlags
{
    FALL_FACE_RIGHT = 0,
    TILE_CHANGE_RIGHT = 0,
    UNKNOWN_STATE = 4,
    TILE_CHANGE_LEFT = 16,
    FALL_FACE_LEFT = 16,
    MOVE_FACE_RIGHT = 32,
    MOVE_FACE_LEFT = 48,
    JUMP_FACE_RIGHT = 128,
    JUMP_FACE_LEFT = 144,
    PUNCH_FACE_RIGHT = 2592,
    PUNCH_FACE_LEFT = 2608,
    PLACE_FACE_RIGHT = 3104,
    PLACE_FACE_LEFT = 3120,
    COLLECT = 16384,
    PICK_ITEM_RIGHT = 16416,
    PICK_ITEM_LEFT = 16432,
    MUSHROOM_FACE_RIGHT = 32768,
    MUSHROOM_FACE_LEFT = 32784


};

struct GameUpdatePacket { //thanks to Inzernal project
    uint8_t type{};
    union {
        uint8_t object_type{};
        uint8_t punch_id;
        uint8_t npc_type;
    };
    union {
        uint8_t count_1{};
        uint8_t jump_count;
        uint8_t build_range;
        uint8_t npc_id;
        uint8_t lost_item_count;
    };
    union {
        uint8_t count_2{};
        uint8_t animation_type;
        uint8_t punch_range;
        uint8_t npc_action;
        uint8_t particle_id;
        uint8_t gained_item_count;
        uint8_t dice_result;
        uint8_t fruit_count;
    };
    union {
        uint32_t net_id{};
        int32_t effect_flags_check;
        int32_t object_change_type;
        int32_t particle_emitter_id;
    };
    union {
        int32_t item{};
        int32_t ping_hash;
        int32_t item_net_id;
        int32_t pupil_color;
        int32_t tiles_length;
    };
    int32_t flags{};
    union {
        float float_var{};
        float water_speed;
        float obj_alt_count;
    };
    union {
        int32_t int_data{};
        int32_t ping_item;
        int32_t elapsed_ms;
        int32_t delay;
        int32_t tile_damage;    
        int32_t item_id;
        int32_t item_speed;
        int32_t effect_flags;
        int32_t object_id;
        int32_t hash;
        int32_t verify_pos;
        int32_t client_hack_type;
        uint32_t dec_item_data_size;
    };
    union {
        float vec_x{};
        float pos_x;
        float accel;
        float punch_range_in;
    };
    union {
        float vec_y{};
        float pos_y;
        float build_range_in;
        float punch_strength;
    };
    union {
        float vec2_x{};
        float dest_x;
        float gravity_in;
        float speed_out;
        float velocity_x;
        float particle_variable;
        float pos2_x;
        int hack_type;
    };
    union {
        float vec2_y{};
        float dest_y;
        float speed_in;
        float gravity_out;
        float velocity_y;
        float particle_alt_id;
        float pos2_y;
        int hack_type2;
    };
    union {
        float particle_rotation{};
        float npc_variable;
    };
    union {
        uint32_t int_x{};
        uint32_t item_id_alt;
        uint32_t eye_shade_color;
    };
    union {
        uint32_t int_y{};
        uint32_t item_count;
        uint32_t eyecolor;
        uint32_t npc_speed;
        uint32_t particle_size_alt;
    };
    uint32_t data_size = 0;
};

struct GameTextPacket
{
    int32_t m_type;
    char m_data;
};