#pragma once
#include "raylib.h"

// --- CONFIG ---
const int TILE_SIZE = 32;
const int TILE_GAP = 4;

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