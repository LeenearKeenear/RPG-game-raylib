#include "../include/item.h"
#include "../include/animation.h"
#include "../include/screen.h"
#include "../include/entities.h"
#include "../include/player.h"
#include "../include/enemy.h"
#include "../include/mapLogic.h"
#include "../lib/raylib/include/raymath.h"
#include <iostream>
#include <vector>

static std::vector<Item> currentItems;
static std::map<std::string, std::vector<Item>> savedMapItems;
std::vector<Item> activeItems;
extern void DrawSmallSprite(TextureAsset slot, Vector2 sheetCoord, Vector2 worldPos, float scale);
extern TileDefinition TileProperty[];

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
    savedMapItems[mapPath] = currentItems;
}

bool LoadItemsforMap(const std::string &mapPath)
{
    if (mapPath.empty())
        return false;

    auto it = savedMapItems.find(mapPath);
    if (it != savedMapItems.end())
    {
        currentItems = it->second;
        return true;
    }
    return false;
}

void ClearItems()
{
    currentItems.clear();
}

void SpawnRandomItem()
{
    if (tilesonMap == nullptr)
        return;
    TraceLog(LOG_INFO, "ITEM: tilesonMap = %p", tilesonMap);

    Vector2 randomPos;
    bool validPos = false;
    int maxAttempts = 1000;
    int attempts = 0;

    float mapWidth = (float)tilesonMap->width * TILE_SIZE;
    float mapHeight = (float)tilesonMap->height * TILE_SIZE;

    // Ambil data collision map sekali
    TiledHelper::CollisionResult mapCollision;
    TiledHelperFunction.TryGetCollisionByLayerName(COLLISION_LAYER_NAME, mapCollision);
    TiledHelper::CollisionResult boundCollision;
    TiledHelperFunction.TryGetCollisionByType("map_bound", boundCollision);

    while (!validPos && attempts < maxAttempts)
    {
        randomPos = {
            (float)GetRandomValue(TILE_SIZE, (int)mapWidth - TILE_SIZE),
            (float)GetRandomValue(TILE_SIZE, (int)mapHeight - TILE_SIZE)};

        // Buat hitbox sementara untuk musuh (asumsi 32x32)
        Rectangle itemHitbox = BuildHitbox(randomPos, 0, 0, TILE_SIZE, TILE_SIZE);

        // Cek apakah posisi ini menabrak wall atau polygon collision
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
        return; // Gagal nemu tempat kosong

    int randomItem = GetRandomValue(1, 2);
    int randomMult = GetRandomValue(1, 3);
    int randomRarity = GetRandomValue(1, 2);

    ItemCategory c;
    if (randomItem == 1)
        c = ITEM_POTION;
    else
        c = ITEM_WEAPON;

    ItemRarity r;
    if (randomRarity == 1)
        r = RARITY_COMMON;
    else
        r = RARITY_RARE;

    activeItems.push_back(SpawnItem(randomPos, c, randomMult, r));
}

Item SpawnItem(Vector2 pos, ItemCategory category, float multiplier, ItemRarity rarity)
{
    Item newItem;
    newItem.category = category;
    newItem.statMultiplier = multiplier;
    newItem.rarity = rarity;
    newItem.isPickedUp = false;
    newItem.position = pos;
    // Tentukan Nama dan Hitbox berdasarkan kategori (Requirement 2 & 3)
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

    // Tentukan nama rarity-nya dulu
    const char *rarityText = "";
    if (rarity == RARITY_COMMON)
        rarityText = "COMMON";
    else if (rarity == RARITY_RARE)
        rarityText = "RARE";

    TraceLog(LOG_INFO, "ITEM: Spawned '%s' at (%.1f, %.1f) with Multiplier: %.2f and rarity '%s'",
             newItem.name.c_str(), pos.x, pos.y, multiplier, rarityText);
    // Masukkan ke dunia (Vector)
    return newItem;
}

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

        // Dalam radius magnet → gerakin item ke player
        if (dist <= magnetRadius)
        {
            Vector2 dir = Vector2Normalize(Vector2Subtract(playerCenter, itemCenter));
            item.position.x += dir.x * itemSpeed * GetFrameTime();
            item.position.y += dir.y * itemSpeed * GetFrameTime();

            // Sync hitbox ke position
            item.hitbox.x = item.position.x;
            item.hitbox.y = item.position.y;
        }

        // Pickup via hitbox collision
        if (CheckCollisionRecs(playerHitbox, item.hitbox))
        {
            item.isPickedUp = true;
            std::cout << "Picked up: " << item.name << std::endl;
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
    // Tentukan koordinat di spritesheet secara manual saja kalau dilarang akses TileProperty
    Vector2 sheetCoord;

    switch (item.category)
    {
    case ITEM_POTION:
        sheetCoord = {7, 8};
        break;
    case ITEM_WEAPON:
        sheetCoord = {6, 4};
        break;
    default:
        sheetCoord = {7, 8};
        break;
    }

    // Panggil fungsi pembungkus tadi
    DrawSmallSprite(TEXTURE_ITEMS, sheetCoord, item.position, 0.5f);
}