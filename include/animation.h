#pragma once
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

// Load/unload knight texture
void LoadKnightTexture();
void UnloadKnightTexture();

// Direction-specific update functions
void UpdatePlayerWalkUp(AnimationPlayer &p);
void UpdatePlayerWalkDown(AnimationPlayer &p);
void UpdatePlayerWalkLeft(AnimationPlayer &p);
void UpdatePlayerWalkRight(AnimationPlayer &p);
void UpdatePlayerIdle(AnimationPlayer &p);
void UpdatePlayerAttack(AnimationPlayer &p);
void UpdatePlayerDeath(AnimationPlayer &p);

// Update animation frames based on state
void UpdateAnimation(AnimationPlayer &p, float dt);

// Draw player sprite
void DrawPlayer(AnimationPlayer &p);

// Buat dipanggil biar gerak
void MoveUp(Vector2 &position, float amount);
void MoveDown(Vector2 &position, float amount);
void MoveLeft(Vector2 &position, float amount);
void MoveRight(Vector2 &position, float amount);