#pragma once
#include "map.h"
#include "animation.h"
#include <vector>
#include <string>
#include <random>

/**
 * @file worldgenenartion.h
 * @brief World Generation Core Module
 *
 * Header ini mendeklarasikan data, struct, enum, utility functions, dan tiga class utama
 * untuk sistem world generation:
 * - WorldGenPools: manajemen pool prefab room dan corridor
 * - WorldGenPrefab: load/unload/rotate satu prefab
 * - WorldGenCanvas: stamping prefab dan corridor ke main map
 */

/*==============================================================================
 * Constants
 *==============================================================================*/

constexpr int WG_GRID_SIZE = 4;            // Jumlah cell per baris/kolom grid worldgen
constexpr int WG_CELL_TILES = 41;          // Ukuran satu cell dalam satuan tile
constexpr int WG_CANVAS_TILES = 164;       // Total tile per sisi canvas (GRID_SIZE * CELL_TILES)
constexpr int WG_PREFAB_LAYER_START = 2;   // Layer awal untuk nge-stamp prefab room
constexpr int WG_CORRIDOR_LAYER_START = 4; // Layer awal untuk nge-stamp corridor
constexpr int WG_TILE_SIZE = FRAME_SIZE;   // Ukuran tile dalam pixel

constexpr const char *SLOT_WORLDGEN_LAYER_NAME = "slot_worldgen";
constexpr const char *EXIT_NORTH_TYPE_OBJECT_NAME = "exit_north";
constexpr const char *EXIT_EAST_TYPE_OBJECT_NAME = "exit_east";
constexpr const char *EXIT_SOUTH_TYPE_OBJECT_NAME = "exit_south";
constexpr const char *EXIT_WEST_TYPE_OBJECT_NAME = "exit_west";

/*==============================================================================
 * Enums
 *==============================================================================*/

/**
 * @brief Arah exit yang tersedia pada suatu room
 *
 * Bitmask: bisa di-combine untuk room dengan multiple exit.
 */
enum ExitDirection
{
    EXIT_NONE = 0,
    EXIT_NORTH = 1 << 0,
    EXIT_EAST = 1 << 1,
    EXIT_SOUTH = 1 << 2,
    EXIT_WEST = 1 << 3,
};

/**
 * @brief Tipe cell dalam grid world generation
 */
enum CellType
{
    CELL_EMPTY = 0,       // Cell kosong / tidak dipakai
    CELL_START = 1,       // Starting room pemain
    CELL_ENEMY = 2,       // Room musuh biasa
    CELL_ENEMY_ELITE = 3, // Room musuh elite
    CELL_TREASURE = 4,    // Room harta karun
    CELL_TRADER = 5,      // Room trader
    CELL_FINISH = 6,      // Room finish
    CELL_BOSS = 7,        // Room boss
    CELL_SPECIAL = 8,     // Room special event
};

/*==============================================================================
 * Structs
 *==============================================================================*/

/**
 * @brief Template room dari file JSON Tiled
 */
struct RoomTemplate
{
    const char *jsonPath; // Path ke file JSON room
    int width;            // Lebar room dalam tile
    int height;           // Tinggi room dalam tile
    int exitMask;         // Bitmask ExitDirection dari room ini
};

/**
 * @brief Satu cell dalam grid world generation
 */
struct WorldCell
{
    CellType type;              // Tipe cell
    int exitMask;               // Bitmask exit yang aktif
    RoomTemplate *roomTemplate; // Template room yang di-assign ke cell ini
};

/**
 * @brief Pool berisi kumpulan prefab dengan flag loaded
 */
struct WeightedPool
{
    std::vector<TilesonMapData *> prefabs; // Kumpulan prefab yang sudah di-load
    bool loaded = false;                   // true jika pool sudah di-load
};

/**
 * @brief Pool corridor — vertical dan horizontal
 */
struct CorridorPool
{
    WeightedPool vertical;   // Pool corridor arah vertical
    WeightedPool horizontal; // Pool corridor arah horizontal
    bool loaded = false;     // true jika corridor pool sudah di-load
};

/**
 * @brief Pool room berdasarkan konfigurasi exit
 *
 * Masing-masing pool berisi room dengan jumlah/jenis exit tertentu:
 * u = 1 exit (bisa utara/selatan/timur/barat)
 * ud = 2 exit (utara + selatan)
 * ur = 2 exit (utara + timur) — L-shape
 * udr = 3 exit (utara + selatan + timur)
 * udrl = 4 exit (semua arah)
 */
struct RoomPool
{
    WeightedPool u;      // 1 exit
    WeightedPool ud;     // 2 exit lurus (utara-selatan)
    WeightedPool ur;     // 2 exit L-shape (utara-timur)
    WeightedPool udr;    // 3 exit (utara-selatan-timur)
    WeightedPool udrl;   // 4 exit
    bool loaded = false; // true jika room pool sudah di-load
};

/*==============================================================================
 * GridMath — Utility Functions
 *==============================================================================*/

/**
 * @brief Utility functions untuk manipulasi grid 2D dan rotasi
 */
namespace GridMath
{
    /**
     * @brief Ubah data tile flat 1D ke grid 2D
     * @param flatData Data tile dalam bentuk vector 1D
     * @param width Lebar grid
     * @param height Tinggi grid
     * @return Grid 2D dengan ukuran height x width
     */
    std::vector<std::vector<int>> ReshapeTo2D(const std::vector<int> &flatData, int width, int height);

    /**
     * @brief Ubah grid 2D kembali ke data tile flat 1D
     * @param grid Grid 2D input
     * @return Vector 1D hasil flatten
     */
    std::vector<int> FlattenTo1D(const std::vector<std::vector<int>> &grid);

    /**
     * @brief Rotasi grid 90 derajat searah jarum jam
     * @param grid Grid 2D input
     * @return Grid hasil rotasi (ukuran width/height tertukar)
     */
    std::vector<std::vector<int>> Rotate90CW(const std::vector<std::vector<int>> &grid);

    /**
     * @brief Rotasi grid dengan kelipatan 90 derajat
     * @param grid Grid 2D input
     * @param degrees Sudut rotasi (0, 90, 180, 270)
     * @return Grid hasil rotasi
     */
    std::vector<std::vector<int>> RotateGrid(const std::vector<std::vector<int>> &grid, int degrees);

    /**
     * @brief Rotasi titik 2D terhadap center dengan kelipatan 90 derajat
     * @param point Titik yang akan dirotasi
     * @param center Pusat rotasi
     * @param degrees Sudut rotasi (0, 90, 180, 270)
     * @return Titik hasil rotasi
     */
    Vector2 RotatePoint(Vector2 point, Vector2 center, int degrees);
}

/*==============================================================================
 * Class: WorldGenPools
 *==============================================================================*/

/**
 * @brief Manajemen pool prefab room dan corridor
 *
 * Bertanggung jawab untuk load/unload seluruh prefab dari folder asset,
 * menyimpannya dalam pool, dan menyediakan akses deterministic
 * ke prefab corridor berdasarkan seed + posisi.
 */
class WorldGenPools
{
public:
    /**
     * @brief Inisialisasi pool dengan seed tertentu
     * @param seed Seed untuk deterministic RNG (default: placeholder)
     */
    WorldGenPools(uint64_t seed = 12345231311);

    /**
     * @brief Load semua room template dari folder
     * @param basePath Base path ke folder room templates
     */
    void LoadRoomPool(const char *basePath);

    /** @brief Unload semua room template dan kosongkan pool */
    void UnloadRoomPool();

    /** @return true jika room pool sudah di-load */
    bool IsRoomLoaded() const;

    /**
     * @brief Load semua corridor prefab dari folder
     * @param basePath Base path ke folder corridor prefabs
     */
    void LoadCorridorPool(const char *basePath);

    /** @brief Unload semua corridor prefab dan kosongkan pool */
    void UnloadCorridorPool();

    /** @return true jika corridor pool sudah di-load */
    bool IsCorridorLoaded() const;

    /**
     * @brief Ambil corridor prefab secara deterministic dari pool
     * @param pool Pool corridor yang akan dipilih (vertical/horizontal)
     * @param tileX Posisi X dalam tile untuk deterministic hash
     * @param tileY Posisi Y dalam tile untuk deterministic hash
     * @param exitTypeHash Hash tipe exit (1-4) untuk salt
     * @return Pointer ke prefab corridor yang terpilih
     */
    TilesonMapData *GetRandomCorridor(WeightedPool &pool, int tileX, int tileY, int exitTypeHash);

    /** @return Reference ke internal CorridorPool */
    CorridorPool &GetCorridorPool();

    RoomPool &GetRoomPool();

private:
    uint64_t worldSeed;        // Seed global untuk deterministic generation
    std::mt19937 wgRng;        // RNG engine (cadangan untuk random pool)
    RoomPool roomPool;         // Pool room templates
    CorridorPool corridorPool; // Pool corridor prefabs

    /**
     * @brief SplitMix64 hash function untuk deterministic selection
     * @param x Input 64-bit
     * @return Output hash 64-bit
     */
    uint64_t SplitMix64(uint64_t x);

    /**
     * @brief Pilih index corridor secara deterministic
     * @param tileX Posisi X tile
     * @param tileY Posisi Y tile
     * @param exitTypeHash Hash tipe exit (1-4)
     * @param poolSize Ukuran pool
     * @return Index terpilih dalam range [0, poolSize)
     */
    int PickCorridorIndex(int tileX, int tileY, int exitTypeHash, int poolSize);
};

/*==============================================================================
 * Class: WorldGenPrefab
 *==============================================================================*/

/**
 * @brief Representasi satu prefab room/corridor yang bisa di-load, dirotate, dan di-unload
 *
 * WorldGenPrefab mengelola siklus hidup TilesonMapData untuk satu prefab.
 * Unload harus dipanggil manual — tidak ada RAII destructor.
 */
class WorldGenPrefab
{
public:
    /** @brief Constructor — data = nullptr */
    WorldGenPrefab();

    /**
     * @brief Load prefab dari file JSON Tiled
     * @param mapPath Path ke file JSON
     */
    void Load(const char *mapPath);

    /** @brief Unload prefab dan bebaskan memory */
    void Unload();

    /** @return Pointer ke internal TilesonMapData */
    TilesonMapData *GetData() const;

    /**
     * @brief Buat prefab baru hasil rotasi dari prefab ini
     * @param tileDegrees Rotasi tile layer (0, 90, 180, 270)
     * @param objectDegrees Rotasi object layer non-obstacle (0, 90, 180, 270)
     * @return WorldGenPrefab baru berisi hasil rotasi (ownership terpisah)
     */
    WorldGenPrefab Rotate(int tileDegrees, int objectDegrees);

private:
    TilesonMapData *data; // Pointer ke data tilemap hasil load
};

/*==============================================================================
 * Class: WorldGenCanvas
 *==============================================================================*/

/**
 * @brief Operasi stamping prefab dan corridor ke main map canvas
 *
 * WorldGenCanvas bekerja pada tilesonMap global dan membutuhkan
 * WorldGenPools untuk akses ke corridor pool saat stamp corridor.
 */
class WorldGenCanvas
{
public:
    /**
     * @param map Pointer ke main map canvas (biasanya tilesonMap global)
     * @param pools Pointer ke instance WorldGenPools
     */
    WorldGenCanvas(TilesonMapData *map, WorldGenPools *pools);

    /**
     * @brief Ambil semua slot worldgen dari layer "slot_worldgen"
     * @return Vector berisi MapObject untuk setiap slot yang ditemukan
     */
    std::vector<MapObject> GetSlots();

    /**
     * @brief Tambah layer baru ke canvas
     * @param extraLayers Jumlah layer yang akan ditambah
     */
    void ExpandLayers(int extraLayers);

    /**
     * @brief Stamp source map ke canvas pada posisi tertentu
     * @param source Map yang akan di-stamp
     * @param offsetX Offset X dalam tile
     * @param offsetY Offset Y dalam tile
     * @param targetLayerOffset Layer awal untuk stamp
     */
    void Stamp(TilesonMapData *source, int offsetX, int offsetY, int targetLayerOffset);

    /**
     * @brief Stamp prefab ke slot worldgen (auto-center)
     * @param prefab Prefab yang akan di-stamp
     * @param slot Slot target
     */
    void StampToSlot(TilesonMapData *prefab, MapObject *slot);

    /**
     * @brief Stamp corridor dari exit object ke border slot
     * @param exitObj Object exit (exit_north/south/east/west)
     * @param slotCol Kolom slot dalam grid worldgen
     * @param slotRow Baris slot dalam grid worldgen
     */
    void StampCorridor(const MapObject &exitObj, int slotCol, int slotRow);

    void StampLayout(const std::vector<std::vector<WorldCell>> &grid,
                     const std::vector<MapObject> &slots,
                     RoomPool &roomPool);

private:
    TilesonMapData *tilesonMap; // Main map canvas — reference ke global
    WorldGenPools *pools;       // Reference ke pools untuk akses corridor
};

class WorldGenLayout
{
public:
    WorldGenLayout(uint64_t seed = 1234523131121131);

    void Generate();

    const std::vector<std::vector<WorldCell>> &GetGrid() const;

private:
    uint64_t worldSeed;
    std::mt19937 wgRng;
    std::vector<std::vector<WorldCell>> grid; // 4x4

    void InitGrid();
    void RunPrims();
    void AssignCellTypes();
    void DebugPrintGrid() const;
};
