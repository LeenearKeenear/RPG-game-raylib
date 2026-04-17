/**
 * @file mapLogic.cpp
 * @brief Implementasi dari Map Logic & Collision Helper Module
 *
 * Implementasi dari class TiledHelper dan fungsi-fungsi helper
 * untuk collision detection, hitbox, point-in-polygon, dll.
 */

#include "../include/mapLogic.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/tileson/tileson.hpp"
#include "../include/map.h"

/*==============================================================================
 * Global Variables
 *==============================================================================*/

/** Global instance TiledHelper - bisa diakses file lain via extern */
TiledHelper TiledHelperFunction;

/*==============================================================================
 * Position Helpers
 *==============================================================================*/

/**
 * @brief Coba dapetin posisi object berdasarkan namanya
 * @param objectName Nama object yang dicari
 * @param outPosition [OUT] Posisi object kalo ketemu
 * @return true kalo object ditemukan
 */
bool TiledHelper::TryGetObjectPositionByName(const std::string &objectName, Vector2 &outPosition)
{
    MapObject *obj = TilesonGetObjectByName(objectName.c_str());
    if (obj == nullptr)
        return false;

    outPosition = {obj->bounds.x, obj->bounds.y};
    return true;
}

/**
 * @brief Coba dapetin posisi object berdasarkan typenya
 * @param objectType Type object yang dicari
 * @param outPosition [OUT] Posisi object pertama yang ketemu
 * @return true kalo ada object dengan type tersebut
 */
bool TiledHelper::TryGetObjectPositionByType(const std::string &objectType, Vector2 &outPosition)
{
    std::vector<MapObject> objs = TilesonGetObjectsByType(objectType.c_str());
    if (objs.empty())
        return false;

    outPosition = {objs[0].bounds.x, objs[0].bounds.y};
    return true;
}

/*==============================================================================
 * Bounds Helpers
 *==============================================================================*/

/**
 * @brief Coba dapetin bounds object berdasarkan typenya
 * @param objectType Type object yang dicari
 * @param outBounds [OUT] Rectangle bounds object pertama yang ketemu
 * @return true kalo ada object dengan type tersebut
 */
bool TiledHelper::TryGetObjectBoundsByType(const std::string &objectType, Rectangle &outBounds)
{
    std::vector<MapObject> objs = TilesonGetObjectsByType(objectType.c_str());
    if (objs.empty())
        return false;

    outBounds = objs[0].bounds;
    return true;
}

/*==============================================================================
 * Collision Helpers
 *==============================================================================*/

/**
 * @brief Coba dapetin collision data berdasarkan layer name
 * @param layerName Nama layer collision (misal "obstacle")
 * @param outCollision [OUT] CollisionResult berisi rects dan polygons
 * @return true kalo layer ditemukan dan ada collision data
 */
bool TiledHelper::TryGetCollisionByLayerName(const std::string &layerName, CollisionResult &outCollision)
{
    std::vector<MapObject> objs = TilesonGetObjectsByLayerName(layerName.c_str());
    if (objs.empty())
        return false;

    // Loop semua object di layer, pisahkan polygon dan rectangle
    for (auto &obj : objs)
    {
        if (obj.hasPolygon)
            outCollision.polygons.push_back(obj.polygonPoints);
        else
            outCollision.rects.push_back(obj.bounds);
    }

    return true;
}

/**
 * @brief Coba dapetin collision data berdasarkan object type
 * @param objectType Type object collision yang dicari
 * @param outCollision [OUT] CollisionResult berisi rects dan polygons
 * @return true kalo ada object dengan type tersebut
 */
bool TiledHelper::TryGetCollisionByType(const std::string &objectType, CollisionResult &outCollision)
{
    std::vector<MapObject> objs = TilesonGetObjectsByType(objectType.c_str());
    if (objs.empty())
        return false;

    // Loop semua object, pisahkan polygon dan rectangle
    for (auto &obj : objs)
    {
        if (obj.hasPolygon)
            outCollision.polygons.push_back(obj.polygonPoints);
        else
            outCollision.rects.push_back(obj.bounds);
    }

    return true;
}

/**
 * @brief Dapetin semua object berdasarkan typenya
 * @param objectType Type object yang dicari
 * @return Vector berisi MapObject yang typenya sesuai
 */
std::vector<MapObject> TiledHelper::GetObjectsByType(const std::string &objectType)
{
    return TilesonGetObjectsByType(objectType.c_str());
}

/*==============================================================================
 * Hitbox Helpers
 *==============================================================================*/

/**
 * @brief Bikin hitbox rectangle berdasarkan posisi dan offset
 * @param position Posisi dasar (biasanya posisi entity)
 * @param offsetX Offset X dari posisi
 * @param offsetY Offset Y dari posisi
 * @param width Lebar hitbox
 * @param height Tinggi hitbox
 * @return Rectangle hitbox yang udah di-offset
 */
Rectangle BuildHitbox(Vector2 position, float offsetX, float offsetY, float width, float height)
{
    Rectangle hitbox;
    hitbox.x = position.x + offsetX;
    hitbox.y = position.y + offsetY;
    hitbox.width = width;
    hitbox.height = height;
    return hitbox;
}

/**
 * @brief Dapetin 4 titik sudut dari rectangle
 * @param rect Rectangle sumber
 * @return Array berisi 4 Vector2 (top-left, top-right, bottom-left, bottom-right)
 */
std::array<Vector2, 4> GetRectangleCorners(const Rectangle &rect)
{
    std::array<Vector2, 4> corners;
    corners[0] = {rect.x, rect.y};                            // kiri atas
    corners[1] = {rect.x + rect.width, rect.y};               // kanan atas
    corners[2] = {rect.x, rect.y + rect.height};              // kiri bawah
    corners[3] = {rect.x + rect.width, rect.y + rect.height}; // kanan bawah
    return corners;
}

/*==============================================================================
 * Point-in-Polygon Helpers
 *==============================================================================*/

/**
 * @brief Cek apakah sebuah titik berada di dalam polygon
 * @param point Titik yang mau dicek
 * @param polygon Daftar titik-titik polygon (berurutan)
 * @return true kalo titik di dalam polygon
 * @note Pake ray casting algorithm (even-odd rule)
 *       Ada epsilon 0.00001f buat menghindari division by zero
 */
bool IsPointInPolygon(Vector2 point, const std::vector<Vector2> &polygon)
{
    bool inside = false;
    int pointCount = (int)polygon.size();

    if (pointCount < 3)
        return false; // polygon minimal butuh 3 titik

    // Ray casting algorithm
    for (int i = 0, j = pointCount - 1; i < pointCount; j = i++)
    {
        const Vector2 &pi = polygon[i];
        const Vector2 &pj = polygon[j];

        // Cek apakah ray dari titik ke kanan memotong edge polygon
        bool intersect = ((pi.y > point.y) != (pj.y > point.y)) &&
                         (point.x < (pj.x - pi.x) * (point.y - pi.y) / ((pj.y - pi.y) + 0.00001f) + pi.x);

        if (intersect)
            inside = !inside; // flip inside flag tiap kali ada intersection
    }

    return inside;
}

/*==============================================================================
 * Collision Check Helpers
 *==============================================================================*/

/**
 * @brief Cek collision antara hitbox dengan kumpulan rectangle
 * @param hitbox Hitbox entity yang mau dicek
 * @param collisionRects Daftar rectangle collision dari map
 * @return true kalo ada tabrakan
 */
bool CheckCollisionAgainstRects(const Rectangle &hitbox, const std::vector<Rectangle> &collisionRects)
{
    for (const auto &rect : collisionRects)
    {
        if (CheckCollisionRecs(hitbox, rect))
            return true;
    }
    return false;
}

/**
 * @brief Cek collision antara hitbox dengan kumpulan polygon
 * @param hitbox Hitbox entity yang mau dicek
 * @param collisionPolygons Daftar polygon collision dari map
 * @return true kalo ada tabrakan (salah satu titik sudut hitbox masuk polygon)
 * @note Cek collision dengan ngecek keempat titik sudut hitbox
 *       apakah ada yang masuk ke dalam polygon
 */
bool CheckCollisionAgainstPolygons(const Rectangle &hitbox, const std::vector<std::vector<Vector2>> &collisionPolygons)
{
    std::array<Vector2, 4> corners = GetRectangleCorners(hitbox);

    for (const auto &polygon : collisionPolygons)
    {
        for (int i = 0; i < 4; i++)
        {
            if (IsPointInPolygon(corners[i], polygon))
                return true;
        }
    }
    return false;
}

/*==============================================================================
 * World Boundary Helpers
 *==============================================================================*/

/**
 * @brief Cek apakah hitbox masih berada di dalam batas dunia
 * @param hitbox Hitbox entity yang mau dicek
 * @param worldWidth Lebar dunia dalam pixel
 * @param worldHeight Tinggi dunia dalam pixel
 * @return true kalo hitbox di dalam batas, false kalo keluar
 */
bool IsWithinWorldBounds(const Rectangle &hitbox, float worldWidth, float worldHeight)
{
    // Cek batas kiri dan atas
    if (hitbox.x < 0.0f)
        return false;
    if (hitbox.y < 0.0f)
        return false;

    // Cek batas kanan dan bawah
    if (hitbox.x + hitbox.width > worldWidth)
        return false;
    if (hitbox.y + hitbox.height > worldHeight)
        return false;

    return true;
}