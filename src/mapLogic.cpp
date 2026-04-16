#include "../include/mapLogic.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/tileson/tileson.hpp"
#include "../include/map.h"

TiledHelper TiledHelperFunction;

// =============================================
// POSITION HELPERS
// =============================================

bool TiledHelper::TryGetObjectPositionByName(const std::string &objectName, Vector2 &outPosition)
{
    MapObject *obj = TilesonGetObjectByName(objectName.c_str());
    if (obj == nullptr)
        return false;

    outPosition = {obj->bounds.x, obj->bounds.y};
    return true;
}

bool TiledHelper::TryGetObjectPositionByType(const std::string &objectType, Vector2 &outPosition)
{
    std::vector<MapObject> objs = TilesonGetObjectsByType(objectType.c_str());
    if (objs.empty())
        return false;

    outPosition = {objs[0].bounds.x, objs[0].bounds.y};
    return true;
}

// =============================================
// BOUNDS HELPERS
// =============================================

bool TiledHelper::TryGetObjectBoundsByType(const std::string &objectType, Rectangle &outBounds)
{
    std::vector<MapObject> objs = TilesonGetObjectsByType(objectType.c_str());
    if (objs.empty())
        return false;

    outBounds = objs[0].bounds;
    return true;
}

// =============================================
// COLLISION HELPERS
// =============================================

bool TiledHelper::TryGetCollisionByLayerName(const std::string &layerName, CollisionResult &outCollision)
{
    std::vector<MapObject> objs = TilesonGetObjectsByLayerName(layerName.c_str());
    if (objs.empty())
        return false;

    for (auto &obj : objs)
    {
        if (obj.hasPolygon)
            outCollision.polygons.push_back(obj.polygonPoints);
        else
            outCollision.rects.push_back(obj.bounds);
    }

    return true;
}

bool TiledHelper::TryGetCollisionByType(const std::string &objectType, CollisionResult &outCollision)
{
    std::vector<MapObject> objs = TilesonGetObjectsByType(objectType.c_str());
    if (objs.empty())
        return false;

    for (auto &obj : objs)
    {
        if (obj.hasPolygon)
            outCollision.polygons.push_back(obj.polygonPoints);
        else
            outCollision.rects.push_back(obj.bounds);
    }

    return true;
}

// buat ngambil properti
std::vector<MapObject> TiledHelper::GetObjectsByType(const std::string &objectType)
{
    return TilesonGetObjectsByType(objectType.c_str());
}

// ================================================================
// Hitbox Helpers
// ================================================================

Rectangle BuildHitbox(Vector2 position, float offsetX, float offsetY, float width, float height)
{
    Rectangle hitbox;
    hitbox.x = position.x + offsetX;
    hitbox.y = position.y + offsetY;
    hitbox.width = width;
    hitbox.height = height;
    return hitbox;
}

std::array<Vector2, 4> GetRectangleCorners(const Rectangle &rect)
{
    std::array<Vector2, 4> corners;
    corners[0] = {rect.x, rect.y};
    corners[1] = {rect.x + rect.width, rect.y};
    corners[2] = {rect.x, rect.y + rect.height};
    corners[3] = {rect.x + rect.width, rect.y + rect.height};
    return corners;
}

// ================================================================
// Point-in-Polygon Helpers
// ================================================================

bool IsPointInPolygon(Vector2 point, const std::vector<Vector2> &polygon)
{
    bool inside = false;
    int pointCount = (int)polygon.size();

    if (pointCount < 3)
        return false;

    for (int i = 0, j = pointCount - 1; i < pointCount; j = i++)
    {
        const Vector2 &pi = polygon[i];
        const Vector2 &pj = polygon[j];

        bool intersect = ((pi.y > point.y) != (pj.y > point.y)) &&
                         (point.x < (pj.x - pi.x) * (point.y - pi.y) / ((pj.y - pi.y) + 0.00001f) + pi.x);

        if (intersect)
            inside = !inside;
    }

    return inside;
}

// ================================================================
// Collision Check Helpers
// ================================================================

bool CheckCollisionAgainstRects(const Rectangle &hitbox, const std::vector<Rectangle> &collisionRects)
{
    for (const auto &rect : collisionRects)
    {
        if (CheckCollisionRecs(hitbox, rect))
            return true;
    }
    return false;
}

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

// ================================================================
// World Boundary Helpers
// ================================================================

bool IsWithinWorldBounds(const Rectangle &hitbox, float worldWidth, float worldHeight)
{
    if (hitbox.x < 0.0f)
        return false;
    if (hitbox.y < 0.0f)
        return false;
    if (hitbox.x + hitbox.width > worldWidth)
        return false;
    if (hitbox.y + hitbox.height > worldHeight)
        return false;
    return true;
}
