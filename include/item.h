#pragma once
#include "../lib/raylib/include/raylib.h"
#include <vector>
#include <string>
#include <map>
#include <random>
#include <algorithm>

/*==============================================================================
 * Enums
 *==============================================================================*/

typedef enum
{
    ITEM_WEAPON,
    ITEM_POTION,
    ITEM_POISON,
    ITEM_ARMOR,
    ITEM_NONE
} ItemCategory;

typedef enum
{
    RARITY_COMMON,
    RARITY_RARE
} ItemRarity;

typedef enum
{
    SPAWN_SIZE_SMALL,
    SPAWN_SIZE_MEDIUM,
    SPAWN_SIZE_LARGE,
    SPAWN_SIZE_XLARGE
} SpawnAreaSize;

/*==============================================================================
 * Structs
 *==============================================================================*/

typedef struct
{
    std::string name;
    ItemCategory category;
    ItemRarity rarity;
    Vector2 position;
    Rectangle hitbox;
    int textureID;
    bool isPickedUp;
    float statMultiplier;
    float spawnTime;
} Item;

typedef struct
{
    std::string name;
    Rectangle bounds;
    SpawnAreaSize sizeClass;
    int minSpawn;
    int maxSpawn;
    bool isActive;
    bool isPolygon;
} SpawnArea;

/*==============================================================================
 * ItemDataManager
 * Handle data item: spawn, save/load per map, pickup state
 *==============================================================================*/
class ItemDataManager
{
public:
    void Init();
    Item CreateItem(Vector2 pos, ItemCategory category, float multiplier, ItemRarity rarity);
    void SpawnItemAtLocation(Vector2 pos); // untuk chest
    void SaveItemsForMap(const std::string &mapPath);
    bool LoadItemsForMap(const std::string &mapPath);
    void ClearItems();

    std::vector<Item> activeItems;

private:
    std::map<std::string, std::vector<Item>> savedMapItems;
};

/*==============================================================================
 * ItemRenderManager
 * Handle render dan update item di world
 *==============================================================================*/
class ItemRenderManager
{
public:
    void Update(std::vector<Item> &items, Vector2 playerCenter,
                Rectangle playerHitbox, float magnetRadius, float itemSpeed);
    void RenderAll(std::vector<Item> &items);
    void Render(Item &item);

private:
    void InitTextures();
};

/*==============================================================================
 * ItemSpawnManager
 * Handle spawn area dari Tiled, kategorisasi, dan distribusi item
 *==============================================================================*/
class ItemSpawnManager
{
public:
    void Init(const std::string &layerName);
    void SpawnAll(std::vector<Item> &activeItems);
    void SpawnRandomItem(std::vector<Item> &activeItems);

private:
    std::vector<SpawnArea> spawnAreas;

    void LoadSpawnAreas(const std::string &layerName);
    void CategorizeAreas();
    SpawnAreaSize ClassifySize(float width, float height);
    void DetermineActiveAreas();
    Vector2 GetRandomPosInArea(const SpawnArea &area);
    unsigned int SeedFromName(const std::string &name);
};

/*==============================================================================
 * Free functions (backward compat, wrapper ke class di atas)
 *==============================================================================*/
void InitItemTextures();
void InitItems();
void SpawnItemWave();
void SpawnRandomItem();

// class instance
extern ItemDataManager itemData;
extern ItemRenderManager itemRender;
extern ItemSpawnManager spawnManager;