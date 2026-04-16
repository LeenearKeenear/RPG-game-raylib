#pragma once

#include "../lib/raylib/include/raylib.h"
#include "../lib/tileson/tileson.hpp"
#include <vector>
#include <string>

struct MapObject;

class TiledHelper
{
public:
    // =============================================
    // POSITION HELPERS (3 methods)
    // =============================================
    
    static bool TryGetObjectPositionByName(const std::string& objectName, Vector2& outPosition);
    static bool TryGetObjectPositionByLayerName(const std::string& layerName, Vector2& outPosition);
    static bool TryGetObjectPositionByType(const std::string& objectType, Vector2& outPosition);
    
    // =============================================
    // BOUNDS HELPERS (3 methods)
    // =============================================
    
    static bool TryGetObjectBoundsByName(const std::string& objectName, Rectangle& outBounds);
    static bool TryGetObjectBoundsByLayerName(const std::string& layerName, Rectangle& outBounds);
    static bool TryGetObjectBoundsByType(const std::string& objectType, Rectangle& outBounds);
    
    // =============================================
    // COLLISION HELPERS (3 methods)
    // =============================================
    
    struct CollisionResult
    {
        std::vector<Rectangle> rects;
        std::vector<std::vector<Vector2>> polygons;
    };
    
    static bool TryGetCollisionByName(const std::string& objectName, CollisionResult& outCollision);
    static bool TryGetCollisionByLayerName(const std::string& layerName, CollisionResult& outCollision);
    static bool TryGetCollisionByType(const std::string& objectType, CollisionResult& outCollision);
};

extern TiledHelper TiledHelperFunction;

// ================================================================
// Hitbox Helpers
// ================================================================

Rectangle BuildHitbox(Vector2 position, float offsetX, float offsetY, float width, float height);
std::array<Vector2, 4> GetRectangleCorners(const Rectangle& rect);

// ================================================================
// Point-in-Polygon Helpers
// ================================================================

bool IsPointInPolygon(Vector2 point, const std::vector<Vector2>& polygon);

// ================================================================
// Collision Check Helpers
// ================================================================

bool CheckCollisionAgainstRects(const Rectangle& hitbox, const std::vector<Rectangle>& collisionRects);
bool CheckCollisionAgainstPolygons(const Rectangle& hitbox, const std::vector<std::vector<Vector2>>& collisionPolygons);

// ================================================================
// World Boundary Helpers
// ================================================================

bool IsWithinWorldBounds(const Rectangle& hitbox, float worldWidth, float worldHeight);