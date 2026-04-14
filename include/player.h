#pragma once
#include <raylib.h>
#include "map.h"
#include "screen.h"
#include "animation.h"
#include "frustum.h"

// ================================================================
// Player Class
// Handle semua behavior player: movement, collision, render
//
// Workflow collision:
// - Collision dibaca dari object layer "collision" di Tiled
// - Setiap Rectangle di layer itu dianggap solid/blocked
// - CanMove() ngecek apakah posisi baru player nabrak salah satu rect itu
//
// Workflow spawn:
// - Spawn point dibaca dari object bernama "spawn" di Tiled
// - Init() otomatis taruh player di posisi spawn
// ================================================================

// nama layer & object di Tiled — sesuaiin kalau beda
#define COLLISION_LAYER_NAME "collision"
#define SPAWN_OBJECT_NAME "spawn"

class Player
{
public:
    // inisialisasi player — load texture, baca spawn & collision dari Tiled
    void Init(void);

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

    // hitung range tile yang visible di layar berdasarkan camera viewport
    // ini adalah inti logic frustum culling — dipake oleh RenderMapCulled()
    TileRange GetVisibleTileRange(void);

private:
    Vector2 Position;
    Vector2 Velocity;
    int TileSize = 32;
    float Speed = 4.0f;
    Texture2D CharTexture;

    // cek apakah posisi baru player nabrak collision rect
    // return false kalau nabrak, true kalau aman
    bool CanMove(Vector2 NewPos);

    // state animasi
    State currentState = IDLE;
    Direction currentDir = DOWN;
    int frame = 0;
    float frameTime = 0.0f;
    float frameSpeed = 0.15f;
    int walkFrameIndex = 0;

    // collision rectangles dari object layer Tiled
    // diisi pas Init() dari TilesonGetObjectsByType(COLLISION_LAYER_NAME)
    std::vector<Rectangle> CollisionRects;
};

// global instance — bisa diakses file lain via extern
extern Player PlayerInstance;