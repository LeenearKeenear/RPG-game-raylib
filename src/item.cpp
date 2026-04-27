#include "../include/item.h"
#include "../include/animation.h"
#include "../include/effects.h"
#include "../include/screen.h"
#include "../include/entities.h"
#include "../include/player.h"
#include "../include/enemy.h"
#include "../include/mapLogic.h"
#include "../lib/raylib/include/raymath.h"
#include <iostream>
#include <vector>
#include <string>
#include <map>

static std::map<std::string, std::vector<Item>> savedMapItems;
std::vector<Item> activeItems;

void InitItemTextures()
{
    LoadTileTexture(TEXTURE_ITEMS, "texture/test.png");
}

void SpawnItemWave()
{
    int jumlahItem = GetRandomValue(1, 10);

    for (int i = 0; i < jumlahItem; i++)
    {
        SpawnRandomItem();
    }

    TraceLog(LOG_INFO, "ITEM: total spawned = %d", (int)activeItems.size());
    for (auto &item : activeItems)
    {
        TraceLog(LOG_INFO, "SPAWNED: %s at (%.1f, %.1f)", item.name.c_str(), item.position.x, item.position.y);
    }
}

void InitItems()
{
    InitItemTextures();
    SpawnItemWave();
}

void SaveItemsForMap(const std::string &mapPath)
{
    if (mapPath.empty())
        return;
    savedMapItems[mapPath] = activeItems;
}

bool LoadItemsforMap(const std::string &mapPath)
{
    if (mapPath.empty())
        return false;

    auto it = savedMapItems.find(mapPath);
    if (it != savedMapItems.end())
    {
        activeItems = it->second;
        return true;
    }
    return false;
}

void ClearItems()
{
    activeItems.clear();
}

void SpawnRandomItem()
{
    if (tilesonMap == nullptr)
        return;

    Vector2 randomPos;
    bool validPos = false;
    int maxAttempts = 100;
    int attempts = 0;

    float mapWidth = (float)tilesonMap->width * TILE_SIZE;
    float mapHeight = (float)tilesonMap->height * TILE_SIZE;

    TiledHelper::CollisionResult mapCollision;
    TiledHelperFunction.TryGetCollisionByLayerName(COLLISION_LAYER_NAME, mapCollision);
    TiledHelper::CollisionResult boundCollision;
    TiledHelperFunction.TryGetCollisionByType("map_bound", boundCollision);

    while (!validPos && attempts < maxAttempts)
    {
        randomPos = {
            (float)GetRandomValue(TILE_SIZE, (int)mapWidth - TILE_SIZE),
            (float)GetRandomValue(TILE_SIZE, (int)mapHeight - TILE_SIZE)};

        Rectangle itemHitbox = BuildHitbox(randomPos, 0, 0, TILE_SIZE, TILE_SIZE);

        bool colRect = CheckCollisionAgainstRects(itemHitbox, mapCollision.rects);
        bool colPoly = CheckCollisionAgainstPolygons(itemHitbox, mapCollision.polygons);
        bool inBounds = IsWithinWorldBounds(itemHitbox, mapWidth, mapHeight);
        bool insideBound = boundCollision.polygons.empty()
                               ? true
                               : !IsPointInPolygon(randomPos, boundCollision.polygons[0]);

        if (!colRect && !colPoly && inBounds && insideBound)
        {
            validPos = true;
        }
        attempts++;
    }

    if (!validPos)
        return;

    ItemCategory c = (GetRandomValue(1, 2) == 1) ? ITEM_POTION : ITEM_WEAPON;
    ItemRarity r = (GetRandomValue(1, 2) == 1) ? RARITY_COMMON : RARITY_RARE;
    activeItems.push_back(SpawnItem(randomPos, c, (float)GetRandomValue(1, 3), r));
}

Item SpawnItem(Vector2 pos, ItemCategory category, float multiplier, ItemRarity rarity)
{
    Item newItem;
    newItem.category = category;
    newItem.statMultiplier = multiplier;
    newItem.rarity = rarity;
    newItem.isPickedUp = false;
    newItem.position = pos;
    
    // Tentukan Nama dan Hitbox berdasarkan kategori
    switch (category)
    {
    case ITEM_WEAPON:
        newItem.name = "Sword";
        newItem.hitbox = {pos.x, pos.y, 32, 32};
        break;
    case ITEM_POTION:
        newItem.name = "Health Potion";
        newItem.hitbox = {pos.x, pos.y, 20, 20};
        break;
    default:
        newItem.name = "Unknown Item";
        newItem.hitbox = {pos.x, pos.y, 16, 16};
        break;
    }

    const char *rarityText = "";
    if (rarity == RARITY_COMMON)
        rarityText = "COMMON";
    else if (rarity == RARITY_RARE)
        rarityText = "RARE";

    TraceLog(LOG_INFO, "ITEM: Spawned '%s' at (%.1f, %.1f) with Multiplier: %.2f and rarity '%s'",
             newItem.name.c_str(), pos.x, pos.y, multiplier, rarityText);

    return newItem;
}

/**
 * @brief Update semua item aktif: magnet pull dan pickup.
 * Menggunakan AnimEffects::LerpTowards untuk smooth magnet pull.
 * Menggunakan Inventory::AddToInventory untuk pickup system.
 * Definisi: src/item.cpp (file ini)
 */
void UpdateItems(Vector2 playerCenter, Rectangle playerHitbox, float magnetRadius, float itemSpeed)
{
    for (auto &item : activeItems)
    {
        if (item.isPickedUp)
            continue;

        Vector2 itemCenter = {
            item.hitbox.x + item.hitbox.width / 2,
            item.hitbox.y + item.hitbox.height / 2};

        float dist = Vector2Distance(playerCenter, itemCenter);

        // Dalam radius magnet → gerakin item ke player (smooth lerp)
        if (dist <= magnetRadius)
        {
            item.position = AnimEffects::LerpTowards(item.position, playerCenter, itemSpeed, GetFrameTime());

            // Sync hitbox ke position
            item.hitbox.x = item.position.x;
            item.hitbox.y = item.position.y;
        }

        // Pickup via hitbox collision
        if (CheckCollisionRecs(playerHitbox, item.hitbox))
        {
            if (Inventory::AddToInventory(PlayerInstance, item)) {
                item.isPickedUp = true;
                std::string logMsg = "Picked up: " + item.name;
                Effects::AddLog(logMsg.c_str());
            }
        }
    }
}

void RenderAllItems()
{
    for (auto &Item : activeItems)
    {
        if (!Item.isPickedUp)
        {
            RenderItems(Item);
        }
    }
}

void RenderItems(Item &item)
{
    Vector2 sheetCoord;
    switch (item.category)
    {
    case ITEM_POTION: sheetCoord = {7, 8}; break;
    case ITEM_WEAPON: sheetCoord = {6, 4}; break;
    default:          sheetCoord = {7, 8}; break;
    }

    DrawSmallSprite(TEXTURE_ITEMS, sheetCoord, item.position, 0.5f);
}