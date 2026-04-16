#pragma once

/**
 * @file map.h
 * @brief Map System & Tileson Integration Module
 *
 * Handle loading, rendering, dan management map dari JSON Tiled.
 * Pake library Tileson buat parse file .tmj (Tiled map JSON).
 * Handle juga collision objects, spawn points, dan frustum culling.
 */

#include "../lib/raylib/include/raylib.h"
#include "../lib/tileson/tileson.hpp"
#include "screen.h"
#include <string>
#include <vector>
#include <map>

/*==============================================================================
 * Global Camera
 *==============================================================================*/

/** Camera2D global - dipake buat rendering world dengan offset/zoom */
extern Camera2D camera;

// ================================================================
// Tileson Map System
// ================================================================

/*==============================================================================
 * MapObject Struct
 *==============================================================================*/

/**
 * @brief Representasi satu object dari object layer Tiled
 * @note Bisa berupa collision rect, spawn point, door, dll
 */
struct MapObject
{
    std::string name;                                 /**< Nama object di Tiled */
    std::string type;                                 /**< Type object (kalo di-set di Tiled) */
    std::string layerName;                            /**< Nama object layer asal di Tiled */
    Rectangle bounds;                                 /**< Bounding box rectangle object */
    std::vector<Vector2> polygonPoints;               /**< Titik polygon/polyline dalam world space */
    bool hasPolygon = false;                          /**< True kalo object punya polygon custom */
    std::map<std::string, tson::Property> properties; /**< Property custom dari Tiled */
};

/*==============================================================================
 * TilesetInfo Struct
 *==============================================================================*/

/**
 * @brief Info satu tileset — texture + metadata buat render
 */
struct TilesetInfo
{
    Texture2D texture; /**< Texture tileset yang udah di-load */
    int cols;          /**< Jumlah kolom tile dalam tileset */
    int spacing;       /**< Jarak antar tile dalam tileset (padding) */
    int firstgid;      /**< Global ID pertama dari tileset ini */
    int lastgid;       /**< Global ID terakhir (firstgid tileset berikutnya - 1) - dipake buat nyari tileset yang bener pas render */
};

/*==============================================================================
 * TilesonMapData Struct
 *==============================================================================*/

/**
 * @brief Semua data map yang udah di-parse dari JSON Tiled
 */
struct TilesonMapData
{
    int width;                         /**< Lebar map dalam satuan tile */
    int height;                        /**< Tinggi map dalam satuan tile */
    int layerCount;                    /**< Jumlah layer dalam map */
    int **tiles;                       /**< Array 2D tile IDs untuk tiap layer */
    std::vector<TilesetInfo> tilesets; /**< Daftar tileset yang dipake map (support multiple tilesets) */
    std::vector<MapObject> Objects;    /**< Daftar object dari semua object layer */
};

/** Global pointer ke data map yang lagi aktif */
extern TilesonMapData *tilesonMap;

/*==============================================================================
 * TileRange Struct
 *==============================================================================*/

/**
 * @brief Hasil kalkulasi frustum: range index tile yang visible di layar
 * @note Dipake buat culling - cuma render tile yang keliatan aja
 */
struct TileRange
{
    int minX; /**< Kolom tile paling kiri yang visible */
    int minY; /**< Baris tile paling atas yang visible */
    int maxX; /**< Kolom tile paling kanan yang visible (exclusive) */
    int maxY; /**< Baris tile paling bawah yang visible (exclusive) */
};

/*==============================================================================
 * Debug Variables
 *==============================================================================*/

/** Jumlah tile yang dirender di frame terakhir - bisa dibaca sistem debug */
extern int lastTilesRendered;

/** Range tile visible di frame terakhir - bisa dibaca sistem debug */
extern TileRange currentVisibleRange;

/*==============================================================================
 * Tiled Layer & Object Name Constants
 *==============================================================================*/

// Nama layer & object di Tiled — sesuaikan kalo beda
// #define COLLISION_LAYER_NAME "map_bound" // (alternatif, sementara di-comment)

#define COLLISION_LAYER_NAME "obstacle" /**< Nama layer buat collision obstacle */
#define OBJECT_LAYER_NAME "object"      /**< Nama layer buat object placement */
#define SPAWN_OBJECT_NAME "spawn"       /**< Nama object buat spawn point player */
#define DOOR_TYPE_OBJECT_NAME "pass"    /**< Type object buat door/pintu */

/*==============================================================================
 * Map Functions
 *==============================================================================*/

/**
 * @brief Load map dari file JSON Tiled
 * @param mapPath Path ke file .tmj (Tiled map JSON)
 * @note Parse file, load texture, dan inisialisasi data map
 */
void LoadMap(const char *mapPath);

/**
 * @brief Render map ke layar
 * @note Pake frustum culling - cuma render tile yang visible di camera
 *       Hasil render diupdate ke lastTilesRendered dan currentVisibleRange
 */
void RenderMap(void);

/**
 * @brief Unload/bersihin data map
 * @note Free memory dan unload texture
 */
void UnloadMap(void);

/**
 * @brief Inisialisasi sistem map
 * @note Dipanggil pas game start
 */
void InitMap(void);

/**
 * @brief Ganti map ke map baru di posisi spawn tertentu
 * @param newMapPath Path ke map baru yang mau di-load
 * @param targetSpawnName Nama spawn point object di map baru
 * @note Unload map lama, load map baru, teleport player ke spawn point yang ditentuin
 */
void SwitchMap(const char *newMapPath, const char *targetSpawnName);

/**
 * @brief Kembali ke map sebelumnya
 * @note Buat fitur go back ke map sebelum SwitchMap()
 */
void GoBack(void);

/*==============================================================================
 * Object Query Functions
 *==============================================================================*/

/**
 * @brief Dapetin semua object dari layer tertentu
 * @param layerName Nama object layer yang mau diambil
 * @return Vector berisi MapObject yang ada di layer tersebut
 */
std::vector<MapObject> TilesonGetObjectsByLayerName(const std::string &layerName);

/**
 * @brief Dapetin semua object dengan type tertentu
 * @param type Type object yang mau dicari (sesuai Tiled)
 * @return Vector berisi MapObject dengan type yang sesuai
 */
std::vector<MapObject> TilesonGetObjectsByType(const std::string &type);

/**
 * @brief Dapetin object berdasarkan nama
 * @param name Nama object yang mau dicari
 * @return Pointer ke MapObject kalo ketemu, nullptr kalo gak ada
 */
MapObject *TilesonGetObjectByName(const std::string &name);