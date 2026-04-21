#pragma once

/**
 * @file maplogic.h
 * @brief Map Logic & Collision Helper Module
 *
 * Nyediain helper functions buat interaksi dengan map:
 * - Posisi object (spawn point, dll)
 * - Bounds object
 * - Collision detection (rectangles & polygons)
 * - Hitbox utilities
 * - Point-in-polygon test
 * - World boundary check
 */

#include "../lib/raylib/include/raylib.h"
#include "../lib/tileson/tileson.hpp"
#include <vector>
#include <string>

struct MapObject;

/*==============================================================================
 * Map Object Index
 *==============================================================================*/

/**
 * @brief Precomputed index buat O(1) lookup MapObject
 * @note Dibangun sekali via BuildMapObjectIndex() setelah LoadMap()
 *       Pointer nunjuk langsung ke element di tilesonMap->Objects
 */
struct MapObjectIndex
{
    std::unordered_map<std::string, MapObject *> byName;
    std::unordered_map<std::string, std::vector<MapObject *>> byType;
    std::unordered_map<std::string, std::vector<MapObject *>> byLayer;
};

/**
 * @brief Build/rebuild index dari tilesonMap->Objects
 * @note Panggil ini setelah LoadMap() selesai
 */
void BuildMapObjectIndex();

/*==============================================================================
 * Object Query Functions
 *==============================================================================*/

/**
 * @brief Dapetin semua object dari layer tertentu
 * @return const reference ke internal vector, zero copy
 */
const std::vector<MapObject *> &TilesonGetObjectsByLayerName(const std::string &layerName);

/**
 * @brief Dapetin semua object dengan type tertentu
 * @return const reference ke internal vector, zero copy
 */
const std::vector<MapObject *> &TilesonGetObjectsByType(const std::string &type);

/**
 * @brief Dapetin object berdasarkan nama
 * @return Pointer ke MapObject kalo ketemu, nullptr kalo gak ada
 */
MapObject *TilesonGetObjectByName(const std::string &name);

/*==============================================================================
 * TiledHelper Class
 *==============================================================================*/

/**
 * @brief Helper class buat interaksi dengan data map dari Tiled
 *
 * Nyediain method-method static buat ngambil data object dari map:
 * - Posisi object berdasarkan nama atau type
 * - Bounds object berdasarkan type
 * - Collision data berdasarkan layer name atau type
 */
class TiledHelper
{
public:
    // =============================================
    // POSITION HELPERS
    // =============================================

    /**
     * @brief Coba dapetin posisi object berdasarkan namanya
     * @param objectName Nama object yang mau dicari
     * @param outPosition [OUT] Posisi object kalo ketemu
     * @return true kalo object ditemukan, false kalo gak ada
     */
    static bool TryGetObjectPositionByName(const std::string &objectName, Vector2 &outPosition);

    /**
     * @brief Coba dapetin posisi object berdasarkan typenya
     * @param objectType Type object yang mau dicari
     * @param outPosition [OUT] Posisi object kalo ketemu (ambil yang pertama)
     * @return true kalo object ditemukan, false kalo gak ada
     */
    static bool TryGetObjectPositionByType(const std::string &objectType, Vector2 &outPosition);

    // =============================================
    // BOUNDS HELPERS
    // =============================================

    /**
     * @brief Coba dapetin bounds object berdasarkan typenya
     * @param objectType Type object yang mau dicari
     * @param outBounds [OUT] Rectangle bounds object kalo ketemu (ambil yang pertama)
     * @return true kalo object ditemukan, false kalo gak ada
     */
    static bool TryGetObjectBoundsByType(const std::string &objectType, Rectangle &outBounds);

    // =============================================
    // COLLISION HELPERS
    // =============================================

    /**
     * @brief Struct buat nampung hasil query collision
     */
    struct CollisionResult
    {
        std::vector<Rectangle> rects;               /**< Daftar rectangle collision */
        std::vector<std::vector<Vector2>> polygons; /**< Daftar polygon collision (masing-masing polygon punya titik-titik) */
    };

    /**
     * @brief Coba dapetin collision data berdasarkan layer name
     * @param layerName Nama layer collision (misal "obstacle")
     * @param outCollision [OUT] CollisionResult berisi rects dan polygons
     * @return true kalo layer ditemukan dan ada collision data, false kalo gak ada
     */
    static bool TryGetCollisionByLayerName(const std::string &layerName, CollisionResult &outCollision);

    /**
     * @brief Coba dapetin collision data berdasarkan object type
     * @param objectType Type object collision yang mau dicari
     * @param outCollision [OUT] CollisionResult berisi rects dan polygons
     * @return true kalo object dengan type tersebut ditemukan, false kalo gak ada
     */
    static bool TryGetCollisionByType(const std::string &objectType, CollisionResult &outCollision);

    /**
     * @brief Dapetin semua object berdasarkan typenya
     * @param objectType Type object yang mau dicari
     * @return Vector berisi MapObject yang typenya sesuai
     */
    static std::vector<MapObject *> GetObjectsByType(const std::string &objectType);
};

/*==============================================================================
 * Global TiledHelper Instance
 *==============================================================================*/

/** Global instance TiledHelper - bisa diakses file lain via extern */
extern TiledHelper TiledHelperFunction;

// ================================================================
// Hitbox Helpers
// ================================================================

/*==============================================================================
 * Hitbox Functions
 *==============================================================================*/

/**
 * @brief Bikin hitbox rectangle berdasarkan posisi dan offset
 * @param position Posisi dasar (biasanya posisi entity)
 * @param offsetX Offset X dari posisi (buat adjustment)
 * @param offsetY Offset Y dari posisi (buat adjustment)
 * @param width Lebar hitbox
 * @param height Tinggi hitbox
 * @return Rectangle hitbox yang udah di-offset
 */
Rectangle BuildHitbox(Vector2 position, float offsetX, float offsetY, float width, float height);

/**
 * @brief Dapetin 4 titik sudut dari rectangle
 * @param rect Rectangle sumber
 * @return Array berisi 4 Vector2 (top-left, top-right, bottom-right, bottom-left)
 */
std::array<Vector2, 4> GetRectangleCorners(const Rectangle &rect);

// ================================================================
// Point-in-Polygon Helpers
// ================================================================

/*==============================================================================
 * Point-in-Polygon Functions
 *==============================================================================*/

/**
 * @brief Cek apakah sebuah titik berada di dalam polygon
 * @param point Titik yang mau dicek
 * @param polygon Daftar titik-titik polygon (berurutan)
 * @return true kalo titik di dalam polygon, false kalo di luar
 * @note Pake ray casting algorithm
 */
bool IsPointInPolygon(Vector2 point, const std::vector<Vector2> &polygon);

// ================================================================
// Collision Check Helpers
// ================================================================

/*==============================================================================
 * Collision Check Functions
 *==============================================================================*/

/**
 * @brief Cek collision antara hitbox dengan kumpulan rectangle
 * @param hitbox Hitbox entity yang mau dicek
 * @param collisionRects Daftar rectangle collision dari map
 * @return true kalo ada tabrakan, false kalo aman
 */
bool CheckCollisionAgainstRects(const Rectangle &hitbox, const std::vector<Rectangle> &collisionRects);

/**
 * @brief Cek collision antara hitbox dengan kumpulan polygon
 * @param hitbox Hitbox entity yang mau dicek
 * @param collisionPolygons Daftar polygon collision dari map
 * @return true kalo ada tabrakan, false kalo aman
 */
bool CheckCollisionAgainstPolygons(const Rectangle &hitbox, const std::vector<std::vector<Vector2>> &collisionPolygons);

// ================================================================
// World Boundary Helpers
// ================================================================

/*==============================================================================
 * World Boundary Functions
 *==============================================================================*/

/**
 * @brief Cek apakah hitbox masih berada di dalam batas dunia
 * @param hitbox Hitbox entity yang mau dicek
 * @param worldWidth Lebar dunia dalam pixel
 * @param worldHeight Tinggi dunia dalam pixel
 * @return true kalo hitbox di dalam batas, false kalo keluar
 */
bool IsWithinWorldBounds(const Rectangle &hitbox, float worldWidth, float worldHeight);