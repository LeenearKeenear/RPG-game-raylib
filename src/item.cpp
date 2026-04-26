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

    while (!validPos && attempts < maxAttempts)
    {
        randomPos = {
            (float)GetRandomValue(TILE_SIZE, (int)mapWidth - TILE_SIZE),
            (float)GetRandomValue(TILE_SIZE, (int)mapHeight - TILE_SIZE)};

        Rectangle itemHitbox = BuildHitbox(randomPos, 0, 0, TILE_SIZE, TILE_SIZE);
        bool colRect = CheckCollisionAgainstRects(itemHitbox, mapCollision.rects);
        bool colPoly = CheckCollisionAgainstPolygons(itemHitbox, mapCollision.polygons);

        if (!colRect && !colPoly)
        {
            validPos = true;
        }
        attempts++;
    }

    if (validPos)
    {
        ItemCategory c = (GetRandomValue(1, 2) == 1) ? ITEM_POTION : ITEM_WEAPON;
        ItemRarity r = (GetRandomValue(1, 2) == 1) ? RARITY_COMMON : RARITY_RARE;
        activeItems.push_back(SpawnItem(randomPos, c, (float)GetRandomValue(1, 3), r));
    }
}

Item SpawnItem(Vector2 pos, ItemCategory category, float multiplier, ItemRarity rarity)
{
    Item newItem;
    newItem.category = category;
    newItem.statMultiplier = multiplier;
    newItem.rarity = rarity;
    newItem.isPickedUp = false;
    newItem.position = pos;
    
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

        if (dist <= magnetRadius)
        {
            Vector2 dir = Vector2Normalize(Vector2Subtract(playerCenter, itemCenter));
            item.position.x += dir.x * itemSpeed * GetFrameTime();
            item.position.y += dir.y * itemSpeed * GetFrameTime();
            item.hitbox.x = item.position.x;
            item.hitbox.y = item.position.y;
        }

        if (CheckCollisionRecs(playerHitbox, item.hitbox))
        {
            item.isPickedUp = true;
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