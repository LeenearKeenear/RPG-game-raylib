#pragma once

/**
 * @file map.h
 * @brief Map System & Tileson Integration Module
 *
 * Header ini mendeklarasikan data dan fungsi utama untuk:
 * - Load dan unload map dari Tiled JSON
 * - Render tile map
 * - Simpan object map dan tileset
 * - Hitung visible tile range untuk culling
 */

#include "../lib/raylib/include/raylib.h"
#include "../lib/tileson/tileson.hpp"
#include "screen.h"
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

/*==============================================================================
 * Global Camera
 *==============================================================================*/

// Camera global untuk rendering world
extern Camera2D camera;

/*==============================================================================
 * MapObject Struct
 *==============================================================================*/

/**
 * @brief Representasi satu object dari object layer Tiled
 */
struct MapObject
{
    int id;                                           // ID unik dari Tiled
    std::string name;                                 // Nama object di Tiled
    std::string type;                                 // Type object di Tiled
    std::string layerName;                            // Nama layer asal object
    Rectangle bounds;                                 // Bounding box object
    std::vector<Vector2> polygonPoints;               // Titik polygon dalam world space
    bool hasPolygon = false;                          // True jika object punya polygon
    std::map<std::string, tson::Property> properties; // Custom properties dari object
};

/*==============================================================================
 * TilesetInfo Struct
 *==============================================================================*/

/**
 * @brief Menyimpan data tileset yang dibutuhkan saat rendering
 */
struct TilesetInfo
{
    Texture2D texture; // Texture tileset yang sudah dimuat
    int cols;          // Jumlah kolom tile dalam tileset
    int spacing;       // Jarak antar tile dalam tileset
    int firstgid;      // Global ID pertama dari tileset
    int lastgid;       // Global ID terakhir dari tileset
};

/*==============================================================================
 * TilesonMapData Struct
 *==============================================================================*/

/**
 * @brief Menyimpan seluruh data map hasil parse Tiled
 */
struct TilesonMapData
{
    int width;                         // Lebar map dalam satuan tile
    int height;                        // Tinggi map dalam satuan tile
    int layerCount;                    // Jumlah tile layer
    int **tiles;                       // Data tile untuk tiap layer
    std::vector<TilesetInfo> tilesets; // Daftar tileset yang dipakai map
    std::vector<MapObject> Objects;    // Daftar object dari object layer
};

// Pointer global ke map yang sedang aktif
extern TilesonMapData *tilesonMap;

/*==============================================================================
 * TileRange Struct
 *==============================================================================*/

/**
 * @brief Range tile yang terlihat di layar
 */
struct TileRange
{
    int minX; // Kolom minimum yang terlihat
    int minY; // Baris minimum yang terlihat
    int maxX; // Kolom maksimum yang terlihat (exclusive)
    int maxY; // Baris maksimum yang terlihat (exclusive)
};

/**
 * @brief Hitung range tile yang terlihat berdasarkan camera
 * @return TileRange untuk kebutuhan frustum culling
 */
TileRange GetVisibleTileRange(void);

/*==============================================================================
 * Debug Variables
 *==============================================================================*/

// Jumlah tile yang dirender pada frame terakhir
extern int lastTilesRendered;

// Range tile visible pada frame terakhir
extern TileRange currentVisibleRange;

/*==============================================================================
 * Tiled Layer & Object Name Constants
 *==============================================================================*/

constexpr const char *COLLISION_LAYER_NAME = "obstacle";       // Nama layer collision obstacle
constexpr const char *OBJECT_LAYER_NAME = "object";            // Nama layer object placement
constexpr const char *TRAP_LAYER_NAME = "trap";                // nama layer trap placement
constexpr const char *ENEMY_SPAWN_OBJECT_NAME = "spawn_enemy"; // Nama objek spesifik untuk spawn musuh
constexpr const char *SPAWN_OBJECT_NAME = "spawn";             // Nama object spawn player
constexpr const char *DOOR_TYPE_OBJECT_NAME = "pass";          // Type object untuk pintu
constexpr const char *CHEST_TYPE_OBJECT_NAME = "chest";        // Type object untuk chest
constexpr const char *SPIKE_TYPE_OBJECT_NAME = "spike";        // Type object unutk spike
constexpr const char *BOMB_TYPE_OBJECT_NAME = "bomb";          // type object untuk bomb

/*==============================================================================
 * Map Functions
 *==============================================================================*/

/**
 * @brief Load map dari file JSON Tiled
 * @param mapPath Path file map yang akan dimuat
 */
void LoadMap(const char *mapPath);

/**
 * @brief Render map ke layar
 */
void RenderMap(void);

/**
 * @brief Bersihkan seluruh data map yang sedang aktif
 */
void UnloadMap(void);

/**
 * @brief Inisialisasi map awal game
 */
void InitMap(void);

/**
 * @brief Pindah ke map baru di titik tujuan tertentu
 * @param newMapPath Path file map tujuan
 * @param targetSpawnName Nama spawn point atau pintu tujuan
 */
void SwitchMap(const char *newMapPath, const char *targetSpawnName);

/**
 * @brief Dapatkan path map yang sedang aktif
 * @return Path map saat ini
 */
const char *GetCurrentMapPath(void);

/**
 * @brief Kembali ke map sebelumnya
 */
void GoBack(void);
