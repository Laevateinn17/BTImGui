#pragma once
#include <string>
#include <vector>
#include "Struct.h"
#include <unordered_map>
#include <map>
#include <iostream>
#include <memory>
#include <filesystem>
#define HashCoord(x, y) (x + (y * 100))
extern ItemContainer itemsDat;
class World
{
public:
	std::string name = "";
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t tileCount = 0;
	std::map<uint32_t, Tile> tiles;
	ThreadSafeMap<uint32_t, Object> objects;
	std::unordered_map<int, Player> players;
	Tile invalid_tile;
	int objectLastIndex = 0;

public:
	World() = default;
	const Tile& GetTile(int x, int y) const
	{
		return tiles.at(HashCoord(x, y));
	}

	Tile& GetTile(int x, int y) 
	{
		if (tiles.find(HashCoord(x, y)) != tiles.end()) return tiles.at(HashCoord(x, y));
		return invalid_tile;
	}
	const Tile& GetTile(const Vector2<int>& pos) const
	{
		if (tiles.find(HashCoord(pos.x, pos.y)) != tiles.end()) return tiles.at(HashCoord(pos.x, pos.y));
		return invalid_tile;
	}

	Tile& GetTile(const Vector2<int>& pos)
	{
		if (tiles.find(HashCoord(pos.x, pos.y))!= tiles.end()) return tiles.at(HashCoord(pos.x, pos.y));
		return invalid_tile;
	}
	std::vector<Tile> GetAdjacentTile(const Vector2<int>& pos)
	{
		if (GetTile(pos).header.fg == invalid_tile.header.fg) return {};
		std::vector<Tile> adj{};
		if (pos.y > 0) adj.push_back(GetTile(pos.x, pos.y - 1));
		if (pos.x < width - 1) adj.push_back(GetTile(pos.x + 1, pos.y));
		if (pos.y < height - 1) adj.push_back(GetTile(pos.x, pos.y + 1));
		if (pos.x > 0) adj.push_back(GetTile(pos.x - 1, pos.y));
		return adj;
	}

	void Serialize(uint8_t* extended)
	{
		try {
			printf("Serializing..\n");
			int counter = 0;
			bool stop = false;
			for (uint32_t y = 0; y < this->height; ++y)
			{
				for (uint32_t x = 0; x < this->width; ++x)
				{
					TileHeader tile;
					tile.fg = *(uint16_t*)extended;
					extended += 2;
					tile.bg = *(uint16_t*)extended;
					extended += 2;
					tile.data = *(uint16_t*)extended;
					extended += 2;
					tile.flag1 = *(uint8_t*)extended++;
					tile.flag2 = *(uint8_t*)extended++;
					tile.pos.x = x;
					tile.pos.y = y;
					if (tile.data)
					{
						uint16_t data = *(uint16_t*)extended;
						if (data == tile.data)
							extended += 2;
					}
					if (tile.flag1)
					{
						//FF 83 CC000000

						if (itemsDat.hasTileExtra(tile.fg))
						{
							uint8_t extraType = *extended;
							++extended;
							switch (extraType)
							{
							case ExtraType::DOOR:
							{
								uint16_t len = *(uint16_t*)extended;
								extended += 2;
								extended += ((uint64_t)len + 1ll);
								break;
							}
							case ExtraType::SIGN:
							{
								uint16_t len = *(uint16_t*)extended;
								extended += 2;
								extended += ((uint64_t)len + 4ll);
								break;
							}
							case ExtraType::LOCK:
							{
								++extended;
								uint32_t userID = *(uint32_t*)extended;
								extended += 4;
								uint8_t adminCount = *extended;
								extended += (((int64_t)adminCount * 4ll) + 12ll);
								break;
							}
							case ExtraType::SEED:
							{
								tile.growTime = itemsDat.itemMap[tile.fg]->growTime;
								tile.elapsedTime = *(uint32_t*)extended;
								tile.snapshotTime = std::time(nullptr);
								extended += 4;
								uint8_t fruitCount = *extended++;
								break;
							}
							case ExtraType::DATA_BEDROCK:
							{
								extended += 21;
								break;
							}
							case ExtraType::VENDING:
							{
								uint32_t data = *(uint32_t*)extended;
								extended += 4;
								uint32_t price = *(uint32_t*)extended;
								extended += 4;
								break;
							}
							case ExtraType::MANNEQUIN:
							{
								uint16_t len = *(uint16_t*)extended;
								extended += 2;
								extended += (uint64_t)len + 5ll;
								extended += (2 * 9); //(9 type of clothing, 2 byte each id)
								break;
							}
							case ExtraType::DISPLAY_SHELF:
							{
								extended += 16;
								break;
							}
							case ExtraType::DISPLAY_BLOCK:
							{
								extended += 4;
								break;
							}
							case ExtraType::SPOTLIGHT:
							{
								break;
							}
							case ExtraType::COUNTRY_FLAG:
							{
								if (tile.fg == 3394)
								{
									uint16_t len = *(uint16_t*)extended;
									extended += 2;
									extended += len;
								}
								break;
							}
							case ExtraType::PORTRAIT:
							{
								uint16_t len = *(uint16_t*)extended;
								extended += 2;
								extended += 26ll + (uint64_t)len;
								break;
							}
							case ExtraType::FORGE:
							{
								extended += 4;
								break;
							}
							case ExtraType::HOWLER:
							{
								break;	//doesnt do anything for now
							}
							case ExtraType::AUDIO_RACK:
							{
								uint16_t len = *(uint16_t*)extended;
								extended += 2;
								extended += ((uint64_t)len + 4ll);
								break;
							}
							case ExtraType::TRANSMUTABOOTH:
							{
								extended += 18;
								break;
							}
							case ExtraType::TOMB_ROBBER:
							{
								break; //doesnt do anything for now
							}
							case ExtraType::PAINTING_EASEL:
							{
								extended += 4;
								uint16_t len = *(uint16_t*)extended;
								extended += 2;
								extended += len;
								break;
							}
							case ExtraType::FISH_PORT:
							{
								++extended;
								uint32_t len = *(uint32_t*)extended;
								extended += 4;
								extended += len * sizeof(len);
								break;
							}
							case ExtraType::VIP_ENTRANCE:
							{
								++extended;
								extended += 4; //owner userid??
								uint32_t len = *(uint32_t*)extended;
								extended += 4;
								extended += len * sizeof(len);
								break;
							}
							case ExtraType::SEWING_MACHINE:
							{
								uint32_t len = *(uint32_t*)extended;
								extended += 4;
								extended += ((uint64_t)len * 4ll);
								break;
							}
							case ExtraType::XENONITE:
							{
								extended += 4;
								++extended;
								break;
							}
							case ExtraType::STORAGE_BOX:
							{
								uint16_t len = *(uint16_t*)extended;
								extended += 2;
								extended += len;
								break;
							}
							case ExtraType::WEATHER_STUFF:
							{
								extended += 9;
								break;
							}
							case ExtraType::HMON:
							{
								extended += 4;
								uint16_t len = *(uint16_t*)extended;
								extended += 2;
								extended += len;
								break;
							}
							case ExtraType::PROVIDER:
							{
								uint32_t growTime = *(uint32_t*)extended;
								extended += 4;
								break;
							}
							case ExtraType::FISH_MOUNT:
							{
								uint16_t len = *(uint16_t*)extended;
								extended += 2;
								extended += ((uint64_t)len + 5ll);
								break;
							}
							case ExtraType::DICE:
							{
								++extended;
								break;
							}
							case ExtraType::WEATHER:
							{
								extended += 4;
								break;
							}
							case ExtraType::VILLAIN:
							{
								uint16_t len = *(uint16_t*)extended;
								extended += 2;
								extended += ((uint64_t)len + 5ll);
								break;
							}
							case ExtraType::ADVENTURE:
							{

								break;
							}
							case ExtraType::STORM_CLOUD:
							{
								extended += 12;
								break;
							}
							case ExtraType::DISAPPEARING_PLAT:
							{
								extended += 4;
								break;
							}
							case ExtraType::BUNNY_EGG:
							{
								extended += 4;
								break;
							}
							case ExtraType::GIVING_TREE:
							{
								++extended;
								extended += 5;
								break;
							}
							case ExtraType::HOME_OVEN:
							{
								extended += 20;
								break;
							}
							case ExtraType::WEATHER_INFINITY:
							{
								extended += 4;
								uint32_t len = *(uint32_t*)extended;
								extended += 4;
								extended += ((uint64_t)len * 4);
								break;
							}
							case ExtraType::CHALLENGE_TIMER:
							{
								break; //doesnt contain anything
							}
							case ExtraType::GROWSCAN:
							{
								++extended;
								break;
							}
							/*case ExtraType::GAME_GEN:
							{
								break;
							}*/
							case ExtraType::CRYSTAL:
							{
								uint16_t len = *(uint16_t*)extended;
								extended += 2;
								extended += len;
								break;
							}
							case ExtraType::FOSSIL_PREP:
							{
								extended += 4;
								break;
							}
							case ExtraType::GAME_GRAVE:
							{
								++extended;
								break;
							}
							case ExtraType::ACHIEVEMENT_BLOCK:
							{
								extended += 5;
								break;
							}
							case ExtraType::STEAM_ORGAN:
							{
								extended += 5;
								break;
							}
							case ExtraType::GAUT:
							{
								uint32_t itemID = *(uint32_t*)extended;
								extended += 4;
								extended += 10;
								break;
							}
							case ExtraType::GUILD_FLAG:
							{
								extended += 17;
								break;
							}
							case ExtraType::STEAM_ENGINE:
							{
								extended += 4;
								break;
							}
							case ExtraType::LOCK_BOT:
							{
								extended += 4;
								break;
							}
							case ExtraType::SSU:
							{
								extended += 4;
								break;
							}
							case ExtraType::PET_TRAINER:
							{
								uint16_t len = (*(uint16_t*)(extended)); extended += 2;
								extended += 32ll + (uint64_t)len;
								break;
							}
							case ExtraType::PET_CAGE:
							{
								uint16_t len = *(uint16_t*)extended;
								extended += 2;
								extended += len;
								extended += 12;
								break;
							}
							case ExtraType::KRANKEN:
							{
								extended += 8;
								break;
							}
							case ExtraType::FRIENDS_ENTRANCE:
							{
								extended += 4;
								break;
							}
							case ExtraType::DNA_PROCESSOR:
							{
								break;
							}
							case ExtraType::SILKWORM:
							{
								++extended;
								break;
							}
							default:
							{
								std::cout << "Unknown extra type\n";
								int except[2];
								except[0] = tile.fg;
								except[1] = extraType;
								break;
							}
							}
						}
					}
					/*else
					{
					int except[2];
					except[0] = tile.fg;
					except[1] = -1;
						throw(except);
					}*/
					tiles[HashCoord(tile.pos.x, tile.pos.y)].header = tile;
					tiles[HashCoord(tile.pos.x, tile.pos.y)].collisionType = itemsDat.itemMap[tile.fg]->collisionType;;
					++counter;
				}
			}
			int objectCount = *(uint32_t*)extended;
			extended += 4;
			this->objectLastIndex = *(uint32_t*)extended;
			extended += 4;
			for (int i = 0; i < objectCount; ++i)
			{
				Object object;
				object.id = *(uint16_t*)extended;
				extended += sizeof(object.id);
				object.x = *(float*)extended;
				extended += sizeof(object.x);
				object.y = *(float*)extended;
				extended += sizeof(object.y);
				object.count = *extended;
				extended += sizeof(object.count);
				object.flag = *extended;
				extended += sizeof(object.flag);
				object.index = *(uint32_t*)extended;
				extended += sizeof(object.index);
				objects.set(object.index, object);
			}
		}
		catch (int exception[2])
		{
			printf("Error serializing Item %d with flag %d\n", exception[0], exception[1]);
		}
		catch (...)
		{
			printf("Unexpected exception..\n");
		}
	}
};

