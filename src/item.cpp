/**
 * @file item.cpp
 * @brief Implementasi Item System
 *
 * File ini berisi implementasi untuk sistem item di dungeon:
 * - ItemDataManager: manajemen data item aktif, spawn, save/load per map
 * - ItemRenderManager: update magnet, pickup detection, dan render item
 * - ItemSpawnManager: baca spawn area dari Tiled, kategorisasi, dan spawn acak per run
 *
 * Free functions dipertahankan sebagai wrapper untuk backward compatibility.
 */

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

extern void DrawSmallSprite(TextureAsset slot, Vector2 sheetCoord, Vector2 worldPos, float scale);
extern TileDefinition TileProperty[];

// instance global ketiga class
ItemDataManager itemData;
ItemRenderManager itemRender;
ItemSpawnManager spawnManager;

/*==============================================================================
 * Free functions (backward compat)
 *==============================================================================*/

/** @brief Load texture item dari spritesheet */
void InitItemTextures()
{
    LoadTileTexture(TEXTURE_ITEMS, "texture/test.png");
}

/**
 * @brief Inisialisasi seluruh item system
 *
 * Load texture, init data manager, load spawn area dari Tiled, spawn item pertama kali.
 */
void InitItems()
{
    InitItemTextures();
    itemData.Init();
    spawnManager.Init(ITEM_LAYER_NAME);
    spawnManager.SpawnAll(itemData.activeItems);
}

/**
 * @brief Reset dan spawn ulang semua item (untuk new run atau reload map)
 *
 * Clear activeItems, re-init spawn area, dan spawn ulang.
 */
void SpawnItemWave()
{
    itemData.activeItems.clear();
    spawnManager.Init(ITEM_LAYER_NAME);
    spawnManager.SpawnAll(itemData.activeItems);
}

/** @brief Spawn item tambahan tanpa clear activeItems */
void SpawnRandomItem()
{
    spawnManager.SpawnAll(itemData.activeItems);
}

/** @brief Getter untuk activeItems */
std::vector<Item> &GetActiveItems() { return itemData.activeItems; }

/*==============================================================================
 * ItemDataManager
 *==============================================================================*/

/** @brief Reset activeItems */
void ItemDataManager::Init()
{
    activeItems.clear();
}

/**
 * @brief Buat item baru dengan properti lengkap
 *
 * Set nama, hitbox, dan rarity berdasarkan category.
 * Log spawn ke console.
 *
 * @param pos Posisi spawn item
 * @param category Jenis item (ITEM_WEAPON / ITEM_POTION)
 * @param multiplier Stat multiplier item
 * @param rarity Rarity item
 * @return Item yang sudah di-setup
 */
Item ItemDataManager::CreateItem(Vector2 pos, ItemCategory category, float multiplier, ItemRarity rarity)
{
    Item newItem;
    newItem.category = category;
    newItem.statMultiplier = multiplier;
    newItem.rarity = rarity;
    newItem.isPickedUp = false;
    newItem.position = pos;
    newItem.spawnTime = (float)GetTime();

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

    const char *rarityText = (rarity == RARITY_COMMON) ? "COMMON" : "RARE";
    TraceLog(LOG_INFO, "ITEM: Spawned '%s' at (%.1f, %.1f) rarity '%s'",
             newItem.name.c_str(), pos.x, pos.y, rarityText);
    return newItem;
}

/**
 * @brief Spawn item secara langsung ke posisi tertentu
 *
 * Dipakai oleh ChestManager saat chest dibuka.
 * Category dipilih random antara ITEM_POTION dan ITEM_WEAPON.
 *
 * @param pos Posisi spawn item
 */
void ItemDataManager::SpawnItemAtLocation(Vector2 pos)
{
    Item newItem;
    newItem.category = (GetRandomValue(0, 1) == 0) ? ITEM_POTION : ITEM_WEAPON;
    newItem.position = pos;
    newItem.isPickedUp = false;
    newItem.spawnTime = (float)GetTime();

    if (newItem.category == ITEM_WEAPON)
    {
        newItem.name = "Sword";
        newItem.hitbox = {pos.x, pos.y, 32, 32};
    }
    else
    {
        newItem.name = "Health Potion";
        newItem.hitbox = {pos.x, pos.y, 20, 20};
    }

    activeItems.push_back(newItem);
    TraceLog(LOG_INFO, "ITEM: Spawned near chest at (%.1f, %.1f)", pos.x, pos.y);
}

/**
 * @brief Simpan state activeItems untuk map tertentu
 * @param mapPath Path map sebagai key penyimpanan
 */
void ItemDataManager::SaveItemsForMap(const std::string &mapPath)
{
    if (mapPath.empty())
        return;
    savedMapItems[mapPath] = activeItems;
}

/**
 * @brief Load state activeItems yang tersimpan untuk map tertentu
 * @param mapPath Path map sebagai key pencarian
 * @return true jika data ditemukan dan di-load, false jika belum ada
 */
bool ItemDataManager::LoadItemsForMap(const std::string &mapPath)
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

/** @brief Hapus semua item aktif */
void ItemDataManager::ClearItems()
{
    activeItems.clear();
}

/*==============================================================================
 * ItemRenderManager
 *==============================================================================*/

/** @brief Load texture item dari spritesheet */
void ItemRenderManager::InitTextures()
{
    LoadTileTexture(TEXTURE_ITEMS, "texture/test.png");
}

/**
 * @brief Update magnet effect dan pickup detection tiap frame
 *
 * Item yang dalam magnetRadius akan bergerak ke arah player.
 * Item yang overlap dengan playerHitbox akan di-pickup (isPickedUp = true).
 * Item yang baru spawn (< 1 detik) diabaikan.
 *
 * @param items Referensi ke activeItems
 * @param playerCenter Posisi center player untuk magnet
 * @param playerHitbox Hitbox player untuk pickup
 * @param magnetRadius Radius magnet dalam pixel
 * @param itemSpeed Kecepatan gerak item saat ditarik magnet
 */
void ItemRenderManager::Update(std::vector<Item> &items, Vector2 playerCenter, Rectangle playerHitbox, float magnetRadius, float itemSpeed)
{
    float currentTime = (float)GetTime();
    for (auto &item : items)
    {
        if (item.isPickedUp)
            continue;
        if (currentTime - item.spawnTime < 1.0f)
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
            std::cout << "Picked up: " << item.name << std::endl;
        }
    }
}

/**
 * @brief Render semua item yang belum di-pickup
 * @param items Referensi ke activeItems
 */
void ItemRenderManager::RenderAll(std::vector<Item> &items)
{
    for (auto &item : items)
    {
        if (!item.isPickedUp)
            Render(item);
    }
}

/**
 * @brief Render satu item berdasarkan category
 *
 * ITEM_POTION = sheet coord (7,8), ITEM_WEAPON = sheet coord (6,4).
 *
 * @param item Item yang akan di-render
 */
void ItemRenderManager::Render(Item &item)
{
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
    DrawSmallSprite(TEXTURE_ITEMS, sheetCoord, item.position, 0.5f);
}

/*==============================================================================
 * ItemSpawnManager
 *==============================================================================*/

/**
 * @brief Generate seed dari nama area untuk randomisasi spawn
 * @param name Nama spawn area dari Tiled
 * @return Seed hasil hash nama
 */
unsigned int ItemSpawnManager::SeedFromName(const std::string &name)
{
    unsigned int seed = 0;
    for (char c : name)
        seed = seed * 31 + static_cast<unsigned int>(c);
    return seed;
}

/**
 * @brief Inisialisasi spawn system untuk map saat ini
 *
 * Clear area lama, load dari layer Tiled, kategorisasi ukuran,
 * tentukan area aktif secara random.
 *
 * @param layerName Nama object layer di Tiled yang berisi spawn area
 */
void ItemSpawnManager::Init(const std::string &layerName)
{
    spawnAreas.clear();
    LoadSpawnAreas(layerName);
    CategorizeAreas();
    DetermineActiveAreas();
}

/**
 * @brief Baca semua spawn area dari object layer Tiled
 * @param layerName Nama layer yang dibaca
 */
void ItemSpawnManager::LoadSpawnAreas(const std::string &layerName)
{
    const auto &objects = TilesonGetObjectsByLayerName(layerName);
    if (objects.empty())
        return;

    for (auto *obj : objects)
    {
        SpawnArea area;
        area.name = obj->name;
        area.bounds = obj->bounds;
        area.isPolygon = obj->hasPolygon;
        area.isActive = false;
        spawnAreas.push_back(area);
    }
}

/**
 * @brief Kategorisasi semua spawn area berdasarkan ukuran
 *
 * Polygon selalu SMALL. Rectangle dikategorisasi berdasarkan sisi terpanjang:
 * <= 128px = SMALL, <= 256px = MEDIUM, <= 384px = LARGE, > 384px = XLARGE.
 * Tiap size class punya range minSpawn-maxSpawn sendiri.
 */
void ItemSpawnManager::CategorizeAreas()
{
    for (auto &area : spawnAreas)
    {
        if (area.isPolygon)
        {
            area.sizeClass = SPAWN_SIZE_SMALL;
            area.minSpawn = 2;
            area.maxSpawn = 3;
            continue;
        }
        area.sizeClass = ClassifySize(area.bounds.width, area.bounds.height);
        switch (area.sizeClass)
        {
        case SPAWN_SIZE_SMALL:
            area.minSpawn = 1;
            area.maxSpawn = 2;
            break;
        case SPAWN_SIZE_MEDIUM:
            area.minSpawn = 2;
            area.maxSpawn = 3;
            break;
        case SPAWN_SIZE_LARGE:
            area.minSpawn = 3;
            area.maxSpawn = 4;
            break;
        case SPAWN_SIZE_XLARGE:
            area.minSpawn = 4;
            area.maxSpawn = 5;
            break;
        }
    }
}

/**
 * @brief Klasifikasi ukuran area berdasarkan dimensi terpanjang
 * @param width Lebar area
 * @param height Tinggi area
 * @return SpawnAreaSize hasil klasifikasi
 */
SpawnAreaSize ItemSpawnManager::ClassifySize(float width, float height)
{
    float longest = (width > height) ? width : height;
    if (longest <= 128.0f)
        return SPAWN_SIZE_SMALL;
    if (longest <= 256.0f)
        return SPAWN_SIZE_MEDIUM;
    if (longest <= 384.0f)
        return SPAWN_SIZE_LARGE;
    return SPAWN_SIZE_XLARGE;
}

/**
 * @brief Tentukan area mana yang aktif secara random tiap run
 *
 * Jumlah area aktif di-random antara 1 sampai total area.
 * Area di-shuffle lalu sejumlah activeCount pertama ditandai aktif.
 */
void ItemSpawnManager::DetermineActiveAreas()
{
    if (spawnAreas.empty())
        return;

    unsigned int seed = static_cast<unsigned int>(time(nullptr));
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> areaDist(1, (int)spawnAreas.size());
    int activeCount = areaDist(rng);

    std::shuffle(spawnAreas.begin(), spawnAreas.end(), rng);
    for (int i = 0; i < activeCount; i++)
        spawnAreas[i].isActive = true;
}

/**
 * @brief Dapatkan posisi random yang aman di dalam spawn area
 *
 * Coba maksimal 100 kali dengan IsPositionSafe check.
 * Fallback ke center area jika semua attempt gagal.
 *
 * @param area SpawnArea target
 * @return Posisi valid dalam area
 */
Vector2 ItemSpawnManager::GetRandomPosInArea(const SpawnArea &area)
{
    int MaxAttempts = 100;

    for (int i = 0; i < MaxAttempts; i++)
    {
        Vector2 pos = {
            (float)GetRandomValue((int)area.bounds.x, (int)(area.bounds.x + area.bounds.width)),
            (float)GetRandomValue((int)area.bounds.y, (int)(area.bounds.y + area.bounds.height))};

        if (IsPositionSafe(pos, TILE_SIZE, TILE_SIZE, 0, 0))
            return pos;
    }

    return {area.bounds.x + area.bounds.width / 2, area.bounds.y + area.bounds.height / 2};
}

/**
 * @brief Spawn semua item ke activeItems berdasarkan area aktif
 *
 * Tiap area aktif spawn item sejumlah random antara minSpawn-maxSpawn.
 * Category dan rarity di-random per item.
 * Log total item yang di-spawn di akhir.
 *
 * @param activeItems Referensi ke vector item yang akan diisi
 */
void ItemSpawnManager::SpawnAll(std::vector<Item> &activeItems)
{
    activeItems.clear();

    for (auto &area : spawnAreas)
    {
        if (!area.isActive)
            continue;

        unsigned int globalSeed = static_cast<unsigned int>(time(nullptr));
        unsigned int nameSeed = SeedFromName(area.name);
        std::mt19937 rng(globalSeed ^ nameSeed);
        std::uniform_int_distribution<int> countDist(area.minSpawn, area.maxSpawn);
        int spawnCount = countDist(rng);

        for (int i = 0; i < spawnCount; i++)
        {
            Vector2 pos = GetRandomPosInArea(area);
            int randomItem = GetRandomValue(1, 2);
            int randomMult = GetRandomValue(1, 3);
            int randomRarity = GetRandomValue(1, 2);

            ItemCategory c = (randomItem == 1) ? ITEM_POTION : ITEM_WEAPON;
            ItemRarity r = (randomRarity == 1) ? RARITY_COMMON : RARITY_RARE;

            activeItems.push_back(itemData.CreateItem(pos, c, (float)randomMult, r));
        }
    }

    TraceLog(LOG_INFO, "ITEM: total spawned = %d", (int)activeItems.size());
}