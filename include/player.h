#pragma once
#include <raylib.h>
#include "map.h"
#include "screen.h"

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
    void Init();   // 🔧 direfaktor: lepas dependency Map & Tileset, baca spawn dari tilesonMap
    void Update(); // 🔧 direfaktor: CanMove() sekarang cek object layer collision
    void Render(); // 🔧 direfaktor: render pake Tileset sendiri, bukan lewat Map
    void PlayerCamera(void); // buat handle camera player
    float GetSpeed() { return Speed; } // buat liat speed player
    void Tick(void); // wrapper function
    Vector2 GetPosition() { return Position; }

private:
    Vector2 Position;
    Vector2 Velocity;
    int TileSize = 32;
    float Speed = 4.0f;
    Texture2D CharTexture;        // 🆕 texture player langsung di sini
    bool CanMove(Vector2 NewPos); // 🔧 parameter ganti ke Vector2, cek collision rectangles

    // collision rectangles dari object layer Tiled
    // diisi pas Init() dari TilesonGetObjectsByType(COLLISION_LAYER_NAME)
    std::vector<Rectangle> CollisionRects; // 🆕
};

// global instance — bisa diakses file lain via extern
extern Player PlayerInstance;
