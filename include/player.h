#pragma once
#include "../lib/raylib/include/raylib.h"
#include "map.h"
#include "screen.h"
#include "animation.h"
#include "input.h"

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
    // inisialisasi player — load texture, baca spawn & collision dari Tiled
    void Init(const char *spawnObjectName = SPAWN_OBJECT_NAME);

    // handle input dan movement per frame, cek collision sebelum apply posisi
    void Update(void);

    // render sprite player di posisi world saat ini
    void Render(void);

    // handle camera follow player dengan clamp ke world bounds
    void PlayerCamera(void);

    // wrapper per frame — dipanggil dari UpdateLogicAll()
    // urutan: Update() → PlayerCamera()
    void Tick(void);

    // getter posisi player dalam pixel
    Vector2 GetPosition() { return Position; }

    // getter speed player — dipake debug panel
    float GetSpeed() { return Speed; }

    // getter apakah player masih hidup
    bool IsAlive() { return !Anim.isDead; }

    // getter hitbox player — dipake collision dan debug panel
    float GetHitboxWidth() { return HitboxWidth; }
    float GetHitboxHeight() { return HitboxHeight; }
    float GetHitboxOffsetX() { return HitboxOffsetX; }
    float GetHitboxOffsetY() { return HitboxOffsetY; }

    // hitung range tile yang visible di layar berdasarkan camera viewport
    // ini adalah inti logic frustum culling — dipake oleh RenderMapCulled()
    TileRange GetVisibleTileRange(void);

    AnimationPlayer Anim;

    bool pendingSwitchMap = false;
    std::string pendingMapPath;
    std::string pendingDoorName;
    bool pendingGoBack = false;

private:
    Vector2 Position;
    Vector2 Velocity;
    int TileSize = 32;
    float Speed = 4.0f;
    Texture2D CharTexture;

    // data animasi player — state machine (idle, walk, attack, dead)

    // ukuran hitbox player bisa diperkecil dari sprite biar movement
    // terasa lebih enak dan gak gampang nyangkut di sudut/object.
    float HitboxWidth = 16.0f;
    float HitboxHeight = 12.0f;
    float HitboxOffsetX = 8.0f;  // makin tinggi makin ke kanan
    float HitboxOffsetY = 14.0f; // makin tinggin makin kebawah

    Rectangle GetPlayerHitboxAtPosition(Vector2 position);

    // cek apakah posisi baru player nabrak collision shape
    // atau keluar dari world boundary
    // return false kalau nabrak / keluar bound, true kalau aman
    bool CanMove(Vector2 NewPos);
    void CheckDoorInteraction(void);

    // handle SPACE berdasarkan context (slot aktif / inventori)
    void HandleSpaceAction(void);

    // revive player — reset state dari DEAD ke IDLE
    void HandleRevive(void);

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
