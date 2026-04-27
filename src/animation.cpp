#include "../include/animation.h"
#include "../lib/raylib/include/raymath.h"

/*==============================================================================
 * Animation Set Data Definitions
 *==============================================================================*/

const AnimationSet PlayerAnimationSet = {
    .configs = {
        [IDLE] = {
            [LEFT]  = {0, 0, 2, 0.5f, true, {0, 1}, 2},
            [RIGHT] = {1, 0, 2, 0.5f, true, {0, 1}, 2},
            [DOWN]  = {2, 0, 2, 0.5f, true, {0, 1}, 2},
            [UP]    = {3, 0, 2, 0.5f, true, {0, 1}, 2}
        },
        [WALK] = {
            [LEFT]  = {0, 0, 4, 0.15f, true, {0, 2, 0, 3}, 4},
            [RIGHT] = {1, 0, 4, 0.15f, true, {0, 2, 0, 3}, 4},
            [DOWN]  = {2, 0, 4, 0.15f, true, {0, 2, 0, 3}, 4},
            [UP]    = {3, 0, 4, 0.15f, true, {0, 2, 0, 3}, 4}
        },
        [ATTACK] = {
            [LEFT]  = {0, 6, 2, 0.15f, false, {0, 1}, 2},
            [RIGHT] = {1, 6, 2, 0.15f, false, {0, 1}, 2},
            [DOWN]  = {2, 4, 2, 0.15f, false, {0, 1}, 2},
            [UP]    = {3, 4, 2, 0.15f, false, {0, 1}, 2}
        },
        [DEAD] = {
            [LEFT]  = {4, 0, 1, 1.0f, false, {0}, 1},
            [RIGHT] = {4, 0, 1, 1.0f, false, {0}, 1},
            [DOWN]  = {4, 0, 1, 1.0f, false, {0}, 1},
            [UP]    = {4, 0, 1, 1.0f, false, {0}, 1}
        }
    }
};

const AnimationSet SlimeAnimationSet = {
    .configs = {
        [IDLE]   = { {0, 0, 2, 0.5f, true, {0, 1}, 2}, {0, 0, 2, 0.5f, true, {0, 1}, 2}, {0, 0, 2, 0.5f, true, {0, 1}, 2}, {0, 0, 2, 0.5f, true, {0, 1}, 2} },
        [WALK]   = { {0, 0, 2, 0.5f, true, {0, 1}, 2}, {0, 0, 2, 0.5f, true, {0, 1}, 2}, {0, 0, 2, 0.5f, true, {0, 1}, 2}, {0, 0, 2, 0.5f, true, {0, 1}, 2} },
        [ATTACK] = { {0, 0, 2, 0.5f, true, {0, 1}, 2}, {0, 0, 2, 0.5f, true, {0, 1}, 2}, {0, 0, 2, 0.5f, true, {0, 1}, 2}, {0, 0, 2, 0.5f, true, {0, 1}, 2} },
        [DEAD]   = { {0, 2, 1, 1.0f, false, {0}, 1},   {0, 2, 1, 1.0f, false, {0}, 1},   {0, 2, 1, 1.0f, false, {0}, 1},   {0, 2, 1, 1.0f, false, {0}, 1} }
    }
};

const AnimationSet SkeletonAnimationSet = {
    .configs = {
        [IDLE]   = { {1, 0, 2, 0.5f, true, {0, 1}, 2}, {1, 0, 2, 0.5f, true, {0, 1}, 2}, {1, 0, 2, 0.5f, true, {0, 1}, 2}, {1, 0, 2, 0.5f, true, {0, 1}, 2} },
        [WALK]   = { {1, 0, 2, 0.5f, true, {0, 1}, 2}, {1, 0, 2, 0.5f, true, {0, 1}, 2}, {1, 0, 2, 0.5f, true, {0, 1}, 2}, {1, 0, 2, 0.5f, true, {0, 1}, 2} },
        [ATTACK] = { {1, 0, 2, 0.5f, true, {0, 1}, 2}, {1, 0, 2, 0.5f, true, {0, 1}, 2}, {1, 0, 2, 0.5f, true, {0, 1}, 2}, {1, 0, 2, 0.5f, true, {0, 1}, 2} },
        [DEAD]   = { {1, 2, 1, 1.0f, false, {0}, 1},   {1, 2, 1, 1.0f, false, {0}, 1},   {1, 2, 1, 1.0f, false, {0}, 1},   {1, 2, 1, 1.0f, false, {0}, 1} }
    }
};

const AnimationSet WolfAnimationSet = {
    .configs = {
        [IDLE]   = { {2, 0, 2, 0.5f, true, {0, 1}, 2}, {2, 0, 2, 0.5f, true, {0, 1}, 2}, {2, 0, 2, 0.5f, true, {0, 1}, 2}, {2, 0, 2, 0.5f, true, {0, 1}, 2} },
        [WALK]   = { {2, 0, 2, 0.5f, true, {0, 1}, 2}, {2, 0, 2, 0.5f, true, {0, 1}, 2}, {2, 0, 2, 0.5f, true, {0, 1}, 2}, {2, 0, 2, 0.5f, true, {0, 1}, 2} },
        [ATTACK] = { {2, 0, 2, 0.5f, true, {0, 1}, 2}, {2, 0, 2, 0.5f, true, {0, 1}, 2}, {2, 0, 2, 0.5f, true, {0, 1}, 2}, {2, 0, 2, 0.5f, true, {0, 1}, 2} },
        [DEAD]   = { {2, 2, 1, 1.0f, false, {0}, 1},   {2, 2, 1, 1.0f, false, {0}, 1},   {2, 2, 1, 1.0f, false, {0}, 1},   {2, 2, 1, 1.0f, false, {0}, 1} }
    }
};

/*==============================================================================
 * Legacy Tile Rendering (RenderTilePNG — masih dipakai oleh beberapa modul)
 * Definisi LoadTileTexture, GetFrame, DrawTileTexture ada di src/tiles.cpp
 *==============================================================================*/

void RenderTilePNG(int pos_x, int pos_y, TileType Type, float Rotation, TextureAsset Slot)
{
    // Mapping TileType ke koordinat di spritesheet
    // Index harus sesuai urutan enum TileType
    static const TileDefinition TileProperty[] = {
        /* TILE_PLAYER_NEW   */ {{3, 2}, false, false},
        /* TILE_CHEST_OPEN   */ {{0, 0}, false, false},
        /* TILE_CHEST_CLOSED */ {{0, 0}, false, false},
        /* TILE_ENEMY_SLIME  */ {{0, 0}, false, true},
        /* TILE_ENEMY_SKELETON*/ {{0, 1}, false, true},
        /* TILE_ENEMY_WOLF   */ {{0, 2}, false, true},
        /* TILE_ITEM_POTION  */ {{7, 8}, false, true},
        /* TILE_WEAPON       */ {{6, 4}, false, true}
    };

    Rectangle Source = {
        (float)(TileProperty[Type].CoordID.x * (TILE_SIZE + TILE_GAP)),
        (float)(TileProperty[Type].CoordID.y * (TILE_SIZE + TILE_GAP)),
        (float)TILE_SIZE,
        (float)TILE_SIZE};

    Rectangle Destination = {(float)pos_x, (float)pos_y, (float)TILE_SIZE, (float)TILE_SIZE};
    Vector2 origin = {0, 0};
    DrawTexturePro(TexturesMap[Slot], Source, Destination, origin, Rotation, WHITE);
}

/*==============================================================================
 * Small Sprite Rendering
 *==============================================================================*/

// Definisi: src/animation.cpp (ini file ini sendiri)
void DrawSmallSprite(TextureAsset slot, Vector2 sheetCoord, Vector2 worldPos, float scale) {
    float smallSize = TILE_SIZE * scale;
    float offset = (TILE_SIZE - smallSize) / 2.0f;

    Rectangle dest = {
        worldPos.x + offset,
        worldPos.y + offset,
        smallSize,
        smallSize
    };

    DrawTileTexture(slot, (int)sheetCoord.x, (int)sheetCoord.y, dest);
}

/*==============================================================================
 * Animation Logic Implementation (Arsitektur akbarazy-2nd)
 *==============================================================================*/

void UpdateAnimation(Animation &anim, float dt)
{
    if (!anim.currentConfig) return;

    anim.timer += dt;
    if (anim.timer >= anim.currentConfig->speed)
    {
        anim.timer = 0;
        anim.walkFrameIndex++;

        if (anim.walkFrameIndex >= anim.currentConfig->patternCount)
        {
            if (anim.currentConfig->loop)
            {
                anim.walkFrameIndex = 0;
            }
            else
            {
                anim.walkFrameIndex = anim.currentConfig->patternCount - 1;
                
                if (anim.state == ATTACK)
                {
                    anim.isAttacking = false;
                    if (anim.animSet) {
                        PlayAnimation(anim, IDLE, anim.direction, *anim.animSet);
                    } else {
                        PlayAnimation(anim, IDLE, anim.direction, PlayerAnimationSet);
                    }
                }
            }
        }
        
        anim.currentFrame = anim.currentConfig->pattern[anim.walkFrameIndex];
    }
}

void DrawAnimation(const Animation &anim, TextureAsset texture, Color tint)
{
    if (!anim.currentConfig) return;

    int frameX = anim.currentConfig->startFrame + anim.currentFrame;
    int row = anim.currentConfig->row;

    Rectangle dest = { anim.position.x, anim.position.y, (float)TILE_SIZE, (float)TILE_SIZE };
    DrawTileTexture(texture, frameX, row, dest, {0,0}, 0.0f, tint);
}

void PlayAnimation(Animation &anim, State newState, Direction newDir, const AnimationSet &set)
{
    if (anim.state == newState && anim.direction == newDir && anim.currentConfig) return;

    anim.state = newState;
    anim.direction = newDir;
    anim.animSet = &set;
    anim.currentConfig = &set.configs[newState][newDir];
    
    anim.timer = 0;
    anim.walkFrameIndex = 0;
    anim.currentFrame = anim.currentConfig->pattern[0];
}

/*==============================================================================
 * Legacy: UpdatePlayerAttack — masih dipakai oleh Player::HandleAction()
 * Definisi: src/animation.cpp (ini file ini sendiri)
 *==============================================================================*/

void UpdatePlayerAttack(Animation &p)
{
    if (!p.isAttacking)
    {
        p.state = ATTACK;
        p.currentFrame = 0;
        p.timer = 0;
        p.isAttacking = true;
    }
}

/*==============================================================================
 * AnimEffects Implementation
 *==============================================================================*/

float AnimEffects::CalculateFadeOut(float timer, float duration)
{
    if (duration <= 0.0f) return 0.0f;
    float alpha = 1.0f - (timer / duration);
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    return alpha;
}

float AnimEffects::CalculateFloatOffset(float currentOffset, float speed, float dt)
{
    return currentOffset - (speed * dt);
}

bool AnimEffects::ShouldBlink(float timer, float frequency)
{
    if (frequency <= 0.0f) return true;
    return (int)(timer * frequency * 10.0f) % 2 == 0;
}

void AnimEffects::ApplyPhysics(Vector2& pos, Vector2& vel, float gravity, float friction, float dt)
{
    // Update posisi berdasarkan velocity
    pos = Vector2Add(pos, Vector2Scale(vel, dt * 60.0f)); // Skala ke 60 FPS
    
    // Terapkan gravitasi
    vel.y += gravity;
    
    // Terapkan friksi horisontal
    vel.x *= friction;
}

Vector2 AnimEffects::LerpTowards(Vector2 current, Vector2 target, float speed, float dt)
{
    Vector2 dir = Vector2Normalize(Vector2Subtract(target, current));
    return Vector2Add(current, Vector2Scale(dir, speed * dt));
}

float AnimEffects::CalculateThrustOffset(float progress, float maxOffset)
{
    return sinf(progress * PI) * maxOffset;
}

float AnimEffects::CalculateSlashRotation(float progress, float startAngle, float sweepAngle)
{
    // Menggunakan easing (Sine Out) agar ayunan terasa lebih natural
    float easedProgress = sinf(progress * PI / 2.0f);
    return startAngle + (easedProgress * sweepAngle);
}
