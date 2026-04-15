#include "../include/animation.h"

// --- SPRITESHEET ---
Texture2D knightTexture;

// --- GET FRAME FROM SPRITESHEET ---
Rectangle GetFrame(int frameX, int frameY) {
    return {
        (float)(frameX * (TILE_SIZE + TILE_GAP)),
        (float)(frameY * (TILE_SIZE + TILE_GAP)),
        (float)TILE_SIZE,
        (float)TILE_SIZE
    };
}

// --- LOAD TEXTURE ---
void LoadKnightTexture() {
    knightTexture = LoadTexture("texture/Knight.png");
}

// --- UNLOAD TEXTURE ---
void UnloadKnightTexture() {
    UnloadTexture(knightTexture);
}

// --- DIRECTION-SPECIFIC UPDATE FUNCTIONS ---

// Update player for walking up
void UpdatePlayerWalkUp(AnimationPlayer &p) {
    p.direction = UP;
    p.state = WALK;
}

// Update player for walking down
void UpdatePlayerWalkDown(AnimationPlayer &p) {
    p.direction = DOWN;
    p.state = WALK;
}

// Update player for walking left
void UpdatePlayerWalkLeft(AnimationPlayer &p) {
    p.direction = LEFT;
    p.state = WALK;
}

// Update player for walking right
void UpdatePlayerWalkRight(AnimationPlayer &p) {
    p.direction = RIGHT;
    p.state = WALK;
}

// Update player for idle (no movement)
void UpdatePlayerIdle(AnimationPlayer &p) {
    p.state = IDLE;
}

// Update player for attack
void UpdatePlayerAttack(AnimationPlayer &p) {
    if (!p.isAttacking) {
        p.state = ATTACK;
        p.frame = 0;
        p.frameTime = 0;
        p.isAttacking = true;
    }
}

// Update player for death
void UpdatePlayerDeath(AnimationPlayer &p) {
    if (!p.isDead) {
        p.state = DEAD;
        p.isDead = true;
        p.frame = 0;
    }
}

// --- UPDATE ANIMATION ---
void UpdateAnimation(AnimationPlayer &p, float dt) {
    // DEAD = no animation (single frame)
    if (p.state == DEAD) {
        p.frame = 0;
        return;
    }

    p.frameTime += dt;

    if (p.state == IDLE) {
        p.frameSpeed = 0.5f;

        if (p.frameTime >= p.frameSpeed) {
            p.frame = (p.frame + 1) % 2;
            p.frameTime = 0;
        }
    }

    else if (p.state == WALK) {
        p.frameSpeed = 0.15f;

        if (p.frameTime >= p.frameSpeed) {
            p.walkFrameIndex = (p.walkFrameIndex + 1) % 4;
            int walkFrames[4] = {0, 2, 0, 3};
            p.frame = walkFrames[p.walkFrameIndex];
            p.frameTime = 0;
        }
    }

    else if (p.state == ATTACK) {
        p.frameSpeed = 0.15f;  // Faster animation for attack

        if (p.frameTime >= p.frameSpeed) {
            p.frame++;
            p.frameTime = 0;

            // Different frame counts for different directions
            int maxFrames = (p.direction == UP || p.direction == DOWN) ? 2 : 4;

            // Hit moment timing differs by direction
            int hitFrame = (p.direction == UP || p.direction == DOWN) ? 1 : 2;
            if (p.frame == hitFrame) {
                TraceLog(LOG_INFO, "HIT!");
            }

            // Reset when reaching max frames
            if (p.frame >= maxFrames) {
                p.isAttacking = false;
                p.state = IDLE;
                p.frame = 0;
            }
        }
    }
}

// --- DRAW PLAYER ---
void DrawPlayer(AnimationPlayer &p) {
    int row = (int)p.direction;
    int frameX = p.frame;

    // Death uses row 4, frame 0
    if (p.state == DEAD) {
        row = 4;
        frameX = 0;
    }
    // Attack uses frames 4-7 in the current direction row
    else if (p.state == ATTACK) {
        frameX += 4;  // Attack frames start at column 4
    }

    Rectangle src = GetFrame(frameX, row);
    DrawTextureRec(knightTexture, src, p.position, WHITE);
}