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
#include "../include/inventory.h"
#include "../include/combat.h"
#include "../include/player.h"
#include "../include/animation.h"
#include "../include/screen.h"
#include "../include/entities.h"
#include "../include/enemy.h"
#include "../include/mapLogic.h"
#include "../lib/raylib/include/raymath.h"
#include <iostream>
#include <vector>

extern void DrawSmallSprite(TextureAsset slot, Vector2 sheetCoord, Vector2 worldPos, float scale);
extern TileDefinition TileProperty[];

// instance global keempat class
ItemDefinitionManager itemDefs;
ItemDataManager itemData;
ItemRenderManager itemRender;
ItemSpawnManager spawnManager;

/*==============================================================================
 * Free functions (backward compat)
 *==============================================================================*/

/** @brief Load texture item dari spritesheet */
void InitItemTextures()
{
    LoadTileTexture(TEXTURE_ITEMS, "assets/textures/test.png");
}

/**
 * @brief Inisialisasi seluruh item system
 *
 * Load texture, init data manager, load spawn area dari Tiled, spawn item pertama kali.
 */
void InitItems()
{
    itemDefs.Init();
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
std::vector<ItemSpawn> &GetActiveItems() { return itemData.activeItems; }

/*==============================================================================
 * ItemDefinitionManager
 *==============================================================================*/

void ItemDefinitionManager::Init()
{
    pool = {
        {0, "Iron Sword", ITEM_WEAPON, {6, 4}, {20, 20}, RARITY_COMMON, false, 1, WeaponData{15.f, 40.f, 16.f, 0.25f, 0.6f, 0.f, 0.f, {8.f, 4.f}, 10.f, ATTACK_THRUST}},

        {1, "Iron Axe", ITEM_WEAPON, {7, 4}, {20, 20}, RARITY_RARE, false, 1, WeaponData{25.f, 48.f, 56.f, 0.5f, 1.8f, 55.f, -95.f, {20.f, 20.f}, 15.f, ATTACK_SLASH}},

        {2, "Health Potion", ITEM_POTION, {7, 8}, {20, 20}, RARITY_COMMON, true, 8, PotionData{20, false}},

        {3, "Mana Bread", ITEM_POTION, {10, 8}, {20, 20}, RARITY_COMMON, true, 8, PotionData{15, true}},
    };
}

const ItemDefinition &ItemDefinitionManager::Get(int id) const
{
    return pool[id];
}

const std::vector<ItemDefinition> &ItemDefinitionManager::GetAll() const
{
    return pool;
}

int ItemDefinitionManager::Count() const
{
    return (int)pool.size();
}

/*==============================================================================
 * ItemDataSpawnManager
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
ItemSpawn ItemDataManager::CreateItem(Vector2 pos, int definitionId)
{
    const ItemDefinition &def = itemDefs.Get(definitionId);
    ItemSpawn item;
    item.definitionId = definitionId;
    item.position = pos;
    item.hitbox = {pos.x - def.hitboxSize.x / 2,
                   pos.y - def.hitboxSize.y / 2,
                   def.hitboxSize.x,
                   def.hitboxSize.y};
    item.isPickedUp = false;
    item.isAdded = false;
    item.spawnTime = (float)GetTime();

    TraceLog(LOG_INFO, "ITEM: Spawned '%s' at (%.1f, %.1f)",
             def.name.c_str(), pos.x, pos.y);
    return item;
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
    std::mt19937 rng(static_cast<unsigned int>(time(nullptr)));
    int defId = spawnManager.PickRandomDefinitionId(rng);
    activeItems.push_back(CreateItem(pos, defId));
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
void ItemRenderManager::Update(std::vector<ItemSpawn> &items, Vector2 playerCenter,
                               Rectangle playerHitbox, float magnetRadius, float itemSpeed)
{

    float currentTime = (float)GetTime();
    for (auto &item : items)
    {
        if (item.isPickedUp)
            continue;
        if (currentTime - item.spawnTime < 1.0f)
            continue;
        if (!Inventory::HasInventorySpace(PlayerInstance))
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
            TraceLog(LOG_INFO, "ITEM: Picked up '%s'",
                     itemDefs.Get(item.definitionId).name.c_str());
        }
    }
}

/**
 * @brief Render semua item yang belum di-pickup
 * @param items Referensi ke activeItems
 */
void ItemRenderManager::RenderAll(std::vector<ItemSpawn> &items)
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
void ItemRenderManager::Render(ItemSpawn &item)
{
    const ItemDefinition &def = itemDefs.Get(item.definitionId);
    Vector2 center = {
        item.hitbox.x + item.hitbox.width / 2,
        item.hitbox.y + item.hitbox.height / 2};
    const float scale = 0.5f;
    float smallSize = TILE_SIZE * scale;
    Vector2 renderPos = {
        center.x - smallSize,
        center.y - smallSize};
    DrawSmallSprite(TEXTURE_ITEMS, def.sheetCoord, renderPos, scale);

    if (item.amount > 1)
    {
        std::string amountText = std::to_string(item.amount);
        int fontSize = 8;
        int textWidth = MeasureText(amountText.c_str(), fontSize);
        Vector2 textPos = {
            center.x - textWidth / 2.0f,
            (center.y + smallSize) - 5.0f};
        DrawText(amountText.c_str(), (int)textPos.x, (int)textPos.y, fontSize, WHITE);
    }
}

/*==============================================================================
 * ItemSpawnManager
 *==============================================================================*/

// Rarity chance dalam persen — total harus 100
static const std::map<ItemRarity, int> RARITY_WEIGHTS = {
    {RARITY_COMMON, 70},
    {RARITY_RARE, 30},
};

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

    std::mt19937 rng(static_cast<unsigned int>(time(nullptr)));
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

// Pilih definitionId random berdasarkan rarity weight
int ItemSpawnManager::PickRandomDefinitionId(std::mt19937 &rng)
{
    // Kumpulkan semua item per rarity
    std::map<ItemRarity, std::vector<int>> byRarity;
    for (const auto &def : itemDefs.GetAll())
        byRarity[def.rarity].push_back(def.id);

    // Roll rarity dulu
    std::uniform_int_distribution<int> rollDist(1, 100);
    int roll = rollDist(rng);

    int cumulative = 0;
    ItemRarity pickedRarity = RARITY_COMMON;
    for (const auto &[rarity, weight] : RARITY_WEIGHTS)
    {
        cumulative += weight;
        if (roll <= cumulative)
        {
            pickedRarity = rarity;
            break;
        }
    }

    // Kalau rarity yang ke-roll gak ada itemnya, fallback ke COMMON
    if (byRarity[pickedRarity].empty())
        pickedRarity = RARITY_COMMON;

    // Pilih random dari item dengan rarity itu
    std::uniform_int_distribution<int> idxDist(0, (int)byRarity[pickedRarity].size() - 1);
    return byRarity[pickedRarity][idxDist(rng)];
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
void ItemSpawnManager::SpawnAll(std::vector<ItemSpawn> &activeItems)
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
            int defId = PickRandomDefinitionId(rng);
            activeItems.push_back(itemData.CreateItem(pos, defId));
        }
    }

    TraceLog(LOG_INFO, "ITEM: total spawned = %d", (int)activeItems.size());
}