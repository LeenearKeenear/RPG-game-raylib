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
 * Structs
 *==============================================================================*/

/**
 * @brief Representasi satu item yang ada di world
 */
typedef struct
{
    std::string name;       // Nama item
    ItemCategory category;  // Kategori item
    ItemRarity rarity;      // Rarity item
    Vector2 position;       // Posisi item di world
    Rectangle hitbox;       // Hitbox item untuk pickup/collision
    int textureID;          // ID texture item
    bool isPickedUp;        // True jika item sudah diambil player
    float statMultiplier;   // Multiplier stat dari item
    float spawnTime;        // Waktu item di-spawn
} Item;

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
    Item CreateItem(Vector2 pos, ItemCategory category, float multiplier, ItemRarity rarity);

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
    std::vector<Item> activeItems;

private:
    // Penyimpanan state item berdasarkan path map
    std::map<std::string, std::vector<Item>> savedMapItems;
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
    void Update(std::vector<Item> &items, Vector2 playerCenter,
                Rectangle playerHitbox, float magnetRadius, float itemSpeed);

    /**
     * @brief Render semua item yang belum diambil
     * @param items Daftar item yang akan dirender
     */
    void RenderAll(std::vector<Item> &items);

    /**
     * @brief Render satu item
     * @param item Item yang akan dirender
     */
    void Render(Item &item);

private:
    /**
     * @brief Load texture item yang dibutuhkan renderer
     */
    void InitTextures();
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
    void SpawnAll(std::vector<Item> &activeItems);

    /**
     * @brief Spawn item random ke daftar item aktif
     * @param activeItems Vector item aktif yang akan diisi
     */
    void SpawnRandomItem(std::vector<Item> &activeItems);

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

// Instance global manager item
extern ItemDataManager itemData;
extern ItemRenderManager itemRender;
extern ItemSpawnManager spawnManager;
