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

TiledHelper TiledHelperFunction;

/**
 * @brief Precomputed index dari tilesonMap->Objects
 * @note Dibangun sekali via BuildMapObjectIndex() setelah LoadMap().
 *       Isinya pointer langsung ke element di tilesonMap->Objects — zero copy.
 *       JANGAN modifikasi tilesonMap->Objects setelah index dibangun.
 */
static MapObjectIndex g_mapIndex;

/**
 * @brief Build index dari semua object di tilesonMap->Objects
 * @note Panggil SEKALI setelah LoadMap() selesai.
 *       Rebuild otomatis kalau dipanggil lagi (misal setelah SwitchMap()).
 */
void BuildMapObjectIndex()
{
    g_mapIndex.byName.clear();
    g_mapIndex.byType.clear();
    g_mapIndex.byLayer.clear();

    for (auto &obj : tilesonMap->Objects)
    {
        // byName: satu nama -> satu object (nama harusnya unik di Tiled)
        if (!obj.name.empty())
            g_mapIndex.byName[obj.name] = &obj;

        // byType: satu type -> banyak object (misal semua "pass" door)
        if (!obj.type.empty())
            g_mapIndex.byType[obj.type].push_back(&obj);

        // byLayer: satu layer -> banyak object (misal semua object di "obstacle")
        if (!obj.layerName.empty())
            g_mapIndex.byLayer[obj.layerName].push_back(&obj);
    }
}

/*==============================================================================
 * Low-level Query Functions
 * Langsung akses g_mapIndex — O(1), tanpa loop, tanpa copy.
 *==============================================================================*/

/**
 * @brief Cari object berdasarkan nama
 * @return Pointer ke object kalo ketemu, nullptr kalo gak ada
 */
MapObject *TilesonGetObjectByName(const std::string &name)
{
    auto it = g_mapIndex.byName.find(name);
    return (it != g_mapIndex.byName.end()) ? it->second : nullptr;
}

/**
 * @brief Ambil semua object dengan type tertentu
 * @return const reference ke internal vector — jangan di-store lama, bisa invalid setelah SwitchMap()
 */
const std::vector<MapObject *> &TilesonGetObjectsByType(const std::string &type)
{
    static const std::vector<MapObject *> empty; // fallback kalo type gak ketemu
    auto it = g_mapIndex.byType.find(type);
    return (it != g_mapIndex.byType.end()) ? it->second : empty;
}

/**
 * @brief Ambil semua object dari layer tertentu
 * @return const reference ke internal vector — jangan di-store lama, bisa invalid setelah SwitchMap()
 */
const std::vector<MapObject *> &TilesonGetObjectsByLayerName(const std::string &layerName)
{
    static const std::vector<MapObject *> empty; // fallback kalo layer gak ketemu
    auto it = g_mapIndex.byLayer.find(layerName);
    return (it != g_mapIndex.byLayer.end()) ? it->second : empty;
}

/*==============================================================================
 * TiledHelper — High-level Helper Functions
 * Wrapper di atas low-level query, return hasil yang lebih spesifik
 * (posisi, bounds, collision) dengan format yang siap pakai.
 *==============================================================================*/

/**
 * @brief Dapetin posisi (x, y) object berdasarkan nama
 * @note Posisi diambil dari sudut kiri atas bounds object
 */
bool TiledHelper::TryGetObjectPositionByName(const std::string &objectName, Vector2 &outPosition)
{
    MapObject *obj = TilesonGetObjectByName(objectName);
    if (!obj)
        return false;
    outPosition = {obj->bounds.x, obj->bounds.y};
    return true;
}

/**
 * @brief Dapetin posisi object pertama yang typenya cocok
 * @note Kalo ada banyak object dengan type sama, yang diambil cuma yang pertama
 */
bool TiledHelper::TryGetObjectPositionByType(const std::string &objectType, Vector2 &outPosition)
{
    const auto &objs = TilesonGetObjectsByType(objectType);
    if (objs.empty())
        return false;
    outPosition = {objs[0]->bounds.x, objs[0]->bounds.y};
    return true;
}

/**
 * @brief Dapetin rectangle bounds object pertama yang typenya cocok
 */
bool TiledHelper::TryGetObjectBoundsByType(const std::string &objectType, Rectangle &outBounds)
{
    const auto &objs = TilesonGetObjectsByType(objectType);
    if (objs.empty())
        return false;
    outBounds = objs[0]->bounds;
    return true;
}

/**
 * @brief Dapetin semua collision data dari layer tertentu
 * @note Object polygon masuk ke outCollision.polygons,
 *       object rectangle biasa masuk ke outCollision.rects
 */
bool TiledHelper::TryGetCollisionByLayerName(const std::string &layerName, CollisionResult &outCollision)
{
    const auto &objs = TilesonGetObjectsByLayerName(layerName);
    if (objs.empty())
        return false;
    for (auto *obj : objs)
    {
        if (obj->hasPolygon)
            outCollision.polygons.push_back(obj->polygonPoints);
        else
            outCollision.rects.push_back(obj->bounds);
    }
    return true;
}

/**
 * @brief Dapetin semua collision data dari object dengan type tertentu
 * @note Sama kayak TryGetCollisionByLayerName tapi filter by type, bukan layer
 */
bool TiledHelper::TryGetCollisionByType(const std::string &objectType, CollisionResult &outCollision)
{
    const auto &objs = TilesonGetObjectsByType(objectType);
    if (objs.empty())
        return false;
    for (auto *obj : objs)
    {
        if (obj->hasPolygon)
            outCollision.polygons.push_back(obj->polygonPoints);
        else
            outCollision.rects.push_back(obj->bounds);
    }
    return true;
}

/**
 * @brief Dapetin owned copy vector semua object dengan type tertentu
 * @note Beda sama TilesonGetObjectsByType yang return reference —
 *       ini return copy jadi aman disimpen lebih lama.
 *       Pake ini kalo lo butuh iterate sambil map mungkin berubah.
 */
std::vector<MapObject *> TiledHelper::GetObjectsByType(const std::string &objectType)
{
    const auto &objs = TilesonGetObjectsByType(objectType);
    return std::vector<MapObject *>(objs.begin(), objs.end());
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