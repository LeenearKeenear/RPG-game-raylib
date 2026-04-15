#pragma once
#include "../lib/raylib/include/raylib.h"
#include "map.h"
#include "screen.h"

// ================================================================
// Player Class
// Handle semua behavior player: movement, collision, render
//
// Workflow collision:
// - Collision dibaca dari object layer "collision" di Tiled
// - Setiap Rectangle / Polygon di layer itu dianggap solid/blocked
// - CanMove() ngecek apakah posisi baru player nabrak salah satu shape itu
//
// Workflow world bound:
// - World boundary custom dibaca dari object layer "map_bound" di Tiled
// - Kalau layer boundary tidak ada / tidak punya polygon,
//   CanMove() fallback ke rectangle ukuran map
//
// Workflow spawn:
// - Spawn point dibaca dari object bernama "spawn" di Tiled
// - Init() otomatis taruh player di posisi spawn
// ================================================================
class Player
{
public:
    void Init(void);
    void Update(void);
    void Render(void);
    void PlayerCamera(void);
    void Tick(void);

    Vector2 GetPosition() { return Position; }
    float GetSpeed() { return Speed; }

    // getter hitbox player — dipake collision dan debug panel
    float GetHitboxWidth() { return HitboxWidth; }
    float GetHitboxHeight() { return HitboxHeight; }
    float GetHitboxOffsetX() { return HitboxOffsetX; }
    float GetHitboxOffsetY() { return HitboxOffsetY; }

    // hitung range tile yang visible di layar berdasarkan camera viewport
    // ini adalah inti logic frustum culling — dipake oleh RenderMapCulled()
    TileRange GetVisibleTileRange(void);
    void RenderHUD(void);

private:
    Vector2 Position;
    Vector2 Velocity;
    int TileSize = 32;
    float Speed = 4.0f;
    Texture2D CharTexture;

    // ukuran hitbox player bisa diperkecil dari sprite biar movement
    // terasa lebih enak dan gak gampang nyangkut di sudut/object.
    float HitboxWidth = 18.0f;
    float HitboxHeight = 13.0f;
    float HitboxOffsetX = 6.0f;
    float HitboxOffsetY = 18.2f;

    // cek apakah posisi baru player nabrak collision shape
    // atau keluar dari world boundary
    // return false kalau nabrak / keluar bound, true kalau aman
    bool CanMove(Vector2 NewPos);

    // collision rectangles dari object layer Tiled
    // diisi pas Init() dari TilesonGetObjectsByLayerName(COLLISION_LAYER_NAME)
    std::vector<Rectangle> CollisionRects;

    // collision polygon dari object layer Tiled
    // diisi pas Init() dari object collision yang punya polygon
    std::vector<std::vector<Vector2>> CollisionPolygons;

    // custom world boundary polygon dari object layer Tiled
    // diisi pas Init() dari TilesonGetObjectsByLayerName(MAP_BOUND_LAYER_NAME)
    // kalau kosong, CanMove() fallback ke rectangle ukuran map
    std::vector<Vector2> WorldBoundaryPolygon;
};

// global instance — bisa diakses file lain via extern
extern Player PlayerInstance;
