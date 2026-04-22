#pragma once

#include "map.h"
#include "mapLogic.h"
#include "animation.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"

// enum buat object state
enum class ObjectState
{
    Closed,
    Open
};

// struct universal buat semua proeperti
struct TileObject
{
    Vector2 position;      // posisi final (sudah dikoreksi ke tile grid)
    Rectangle bounds;      // bounds asli dari MapObject
    ObjectState state;
    std::string name;      // nama dari Tiled (buat identifikasi)
};

// entry buat nge spawn semua jenis tile
void SpawnObject(void);

// class buat chest
class ChestManager
{
public:
    void SpawnChests(const std::vector<MapObject*>& chestObjects);
    void Interact(const std::string& chestName);
    void Render();
    void Clear();

private:
    std::vector<TileObject> chests;

    TileObject* FindChest(const std::string& name);
    void TriggerLoot(const std::string& chestName); // placeholder
};

extern ChestManager chestManager;