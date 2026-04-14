#pragma once

// ================================================================
// Animation System
// Handle semua animasi sprite untuk player, enemy, dan entity lain.
//
// Semua logic animasi (frame switching, timing, direction)
// dipusatin di sini biar gak nyebar ke mana-mana.
//
// TODO (pindahan dari map.h / map.cpp):
// - LoadTileTexture()
// - RenderTilePNG()
// - TileDefinition struct
// - TileType enum
// - TileCoordinate struct
// - TextureAsset enum
// - TexturesMap array
// - MAX_TEXTURES define
// - TILE_SIZE, TILE_GAP define
//
// Setelah dipindah, update semua include dan pemanggilan
// di map.cpp, player.cpp, entities.cpp, dll.
// ================================================================

#include "raylib.h"

// --- CONFIG ---
// TILE_SIZE dan TILE_GAP juga didefinisikan di map.h sebagai #define.
// Pakai #ifndef supaya gak konflik kalau map.h sudah di-include duluan.
#ifndef TILE_SIZE
#define TILE_SIZE 32
#endif
#ifndef TILE_GAP
#define TILE_GAP 4
#endif

// --- ENUMS ---
enum State {
    IDLE,
    WALK,
    ATTACK,
    DEAD
};

enum Direction {
    LEFT,
    RIGHT,
    DOWN,
    UP
};

// --- ANIMATION PLAYER STRUCT ---
struct AnimationPlayer {
    Vector2 position;

    State state;
    Direction direction;

    int frame;
    float frameTime;
    float frameSpeed;

    int walkFrameIndex;

    bool isAttacking;
    bool isDead;
};

// --- FUNCTION DECLARATIONS ---
// Get frame rectangle from spritesheet
Rectangle GetFrame(int frameX, int frameY);

// Update player input and state for animation demo purposes
void UpdatePlayer(AnimationPlayer &p);

// Update animation frames based on state
void UpdateAnimation(AnimationPlayer &p, float dt);

// Draw player sprite
void DrawPlayer(AnimationPlayer &p, Texture2D texture);

// Movement helpers for external use (e.g. Player.cpp can call these)
void MoveUp(Vector2 &position, float amount);
void MoveDown(Vector2 &position, float amount);
void MoveLeft(Vector2 &position, float amount);
void MoveRight(Vector2 &position, float amount);