#pragma once

/**
 * @file item.h
 * @brief Item System Module
 *
 * Header ini mendeklarasikan data dan fungsi utama untuk:
 * - Menyimpan data item dan spawn area
 * - Spawn item dari object layer Tiled
 * - Render item di world
 * - Update pickup dan magnet item
 * - Save/load state item per map
 */

#include "../lib/raylib/include/raylib.h"
#include <vector>
#include <string>
#include <map>
#include <random>
#include <algorithm>
#include <variant>

/*==============================================================================
 * Enums
 *==============================================================================*/

/**
 * @brief Kategori utama item
 */
typedef enum
{
    ITEM_WEAPON, // Item senjata
    ITEM_POTION, // Item potion/healing
    ITEM_POISON, // Item poison
    ITEM_ARMOR,  // Item armor
    ITEM_NONE    // Tidak ada item
} ItemCategory;

/**
 * @brief Tingkat rarity item
 */
typedef enum
{
    RARITY_COMMON, // Item biasa
    RARITY_RARE    // Item langka
} ItemRarity;

/**
 * @brief Klasifikasi ukuran area spawn item
 */
typedef enum
{
    SPAWN_SIZE_SMALL,  // Area spawn kecil
    SPAWN_SIZE_MEDIUM, // Area spawn sedang
    SPAWN_SIZE_LARGE,  // Area spawn besar
    SPAWN_SIZE_XLARGE  // Area spawn sangat besar
} SpawnAreaSize;

/*==============================================================================
 * Behavior Structs per Category
 *==============================================================================*/

// Pindah dari combat.h — dipakai sebagai bagian dari WeaponData
enum AttackType
{
    ATTACK_SLASH,
    ATTACK_THRUST
};

struct WeaponData
{
    float damage;
    float reach;
    float breadth;
    float duration;
    float knockbackForce;
    float startAngleOffset;
    float sweepAngle;
    Vector2 centerOffset;
    float manaCost;
    AttackType attackType;
};

struct PotionData
{
    int healValue;
    bool isMana;
};

struct ArmorData
{
    float defense;
};

/*==============================================================================
 * ItemDefinition — single source of truth
 *==============================================================================*/

struct ItemDefinition
{
    int id;
    std::string name;
    ItemCategory category;
    Vector2 sheetCoord;
    Vector2 hitboxSize;
    ItemRarity rarity;
    std::variant<WeaponData, PotionData, ArmorData> data;
};

/*==============================================================================
 * Structs
 *==============================================================================*/

struct ItemSpawn
{
    int definitionId;
    Vector2 position;
    Rectangle hitbox;
    bool isPickedUp;
    bool isAdded;
    float spawnTime;
    int amount = 1;
};

struct InventoryItem
{
    int definitionId = -1; // -1 = slot kosong
    int amount = 0;
};

/**
 * @brief Representasi satu area spawn item dari Tiled
 */
typedef struct
{
    std::string name;        // Nama area spawn dari Tiled
    Rectangle bounds;        // Bounding box area spawn
    SpawnAreaSize sizeClass; // Kategori ukuran area spawn
    int minSpawn;            // Jumlah minimum item yang di-spawn
    int maxSpawn;            // Jumlah maksimum item yang di-spawn
    bool isActive;           // True jika area dipakai untuk spawn pada run ini
    bool isPolygon;          // True jika area berasal dari object polygon
} SpawnArea;

/*==============================================================================
 * ItemDefinitionManager
 *==============================================================================*/

class ItemDefinitionManager
{
public:
    void Init();
    const ItemDefinition &Get(int id) const;
    const std::vector<ItemDefinition> &GetAll() const;
    int Count() const;

private:
    std::vector<ItemDefinition> pool;
};

/*==============================================================================
 * ItemDataManager
 *==============================================================================*/

/**
 * @brief Mengelola data item aktif, spawn langsung, dan save/load per map
 */
class ItemDataManager
{
public:
    /**
     * @brief Inisialisasi data item
     */
    void Init();

    /**
     * @brief Buat item baru dengan properti tertentu
     * @param pos Posisi spawn item
     * @param category Kategori item
     * @param multiplier Multiplier stat item
     * @param rarity Rarity item
     * @return Item yang sudah dibuat
     */

    ItemSpawn CreateItem(Vector2 pos, int definitionId);
    /**
     * @brief Spawn item langsung di posisi tertentu
     * @param pos Posisi spawn item
     */
    void SpawnItemAtLocation(Vector2 pos);

    /**
     * @brief Simpan state item aktif untuk map tertentu
     * @param mapPath Path map sebagai key penyimpanan
     */
    void SaveItemsForMap(const std::string &mapPath);

    /**
     * @brief Load state item aktif untuk map tertentu
     * @param mapPath Path map sebagai key pencarian
     * @return true jika data map ditemukan, false jika belum ada
     */
    bool LoadItemsForMap(const std::string &mapPath);

    /**
     * @brief Hapus semua item aktif
     */
    void ClearItems();

    // Daftar item yang sedang aktif di world
    std::vector<ItemSpawn> activeItems;

private:
    // Penyimpanan state item berdasarkan path map
    std::map<std::string, std::vector<ItemSpawn>> savedMapItems;
};

/*==============================================================================
 * ItemRenderManager
 *==============================================================================*/

/**
 * @brief Mengelola update dan render item di world
 */
class ItemRenderManager
{
public:
    /**
     * @brief Update magnet item dan deteksi pickup
     * @param items Daftar item yang akan di-update
     * @param playerCenter Posisi tengah player
     * @param playerHitbox Hitbox player untuk deteksi pickup
     * @param magnetRadius Radius tarikan item ke player
     * @param itemSpeed Kecepatan item saat tertarik ke player
     */
    void Update(std::vector<ItemSpawn> &items, Vector2 playerCenter,
                Rectangle playerHitbox, float magnetRadius, float itemSpeed);

    /**
     * @brief Render semua item yang belum diambil
     * @param items Daftar item yang akan dirender
     */
    void RenderAll(std::vector<ItemSpawn> &items);

    /**
     * @brief Render satu item
     * @param item Item yang akan dirender
     */
    void Render(ItemSpawn &item);

private:
};

/*==============================================================================
 * ItemSpawnManager
 *==============================================================================*/

/**
 * @brief Mengelola spawn area dari Tiled dan distribusi item
 */
class ItemSpawnManager
{
public:
    /**
     * @brief Inisialisasi spawn manager dari layer Tiled tertentu
     * @param layerName Nama object layer yang berisi area spawn item
     */
    void Init(const std::string &layerName);

    /**
     * @brief Spawn semua item berdasarkan area aktif
     * @param activeItems Vector item aktif yang akan diisi
     */
    void SpawnAll(std::vector<ItemSpawn> &activeItems);

    int PickRandomDefinitionId(std::mt19937 &rng);

private:
    // Daftar area spawn item yang dibaca dari Tiled
    std::vector<SpawnArea> spawnAreas;

    /**
     * @brief Load spawn area dari object layer Tiled
     * @param layerName Nama layer yang akan dibaca
     */
    void LoadSpawnAreas(const std::string &layerName);

    /**
     * @brief Kategorisasi area berdasarkan ukuran
     */
    void CategorizeAreas();

    /**
     * @brief Klasifikasikan ukuran area spawn
     * @param width Lebar area
     * @param height Tinggi area
     * @return Kategori ukuran spawn area
     */
    SpawnAreaSize ClassifySize(float width, float height);

    /**
     * @brief Tentukan area spawn yang aktif untuk run saat ini
     */
    void DetermineActiveAreas();

    /**
     * @brief Ambil posisi random di dalam area spawn
     * @param area Area spawn target
     * @return Posisi random di dalam area
     */
    Vector2 GetRandomPosInArea(const SpawnArea &area);

    /**
     * @brief Buat seed random dari nama area
     * @param name Nama area spawn
     * @return Seed hasil hash nama
     */
    unsigned int SeedFromName(const std::string &name);
};

/*==============================================================================
 * Free functions (backward compat, wrapper ke class di atas)
 *==============================================================================*/

/**
 * @brief Load texture item
 */
void InitItemTextures();

/**
 * @brief Inisialisasi seluruh sistem item
 */
void InitItems();

/**
 * @brief Reset dan spawn ulang item untuk map/run saat ini
 */
void SpawnItemWave();

/**
 * @brief Spawn item random melalui spawn manager
 */
void SpawnRandomItem();

std::vector<ItemSpawn> &GetActiveItems();

// Instance global manager item
extern ItemDataManager itemData;
extern ItemRenderManager itemRender;
extern ItemSpawnManager spawnManager;
extern ItemDefinitionManager itemDefs;
