#include "../include/animation.h"

// --- Definisi Data Animasi ---

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
 * Animation Logic Implementation
 *==============================================================================*/

void DrawSmallSprite(TextureAsset slot, Vector2 sheetCoord, Vector2 worldPos, float scale) {
    Rectangle source = GetFrame((int)sheetCoord.x, (int)sheetCoord.y);

    float smallSize = TILE_SIZE * scale;
    float offset = (TILE_SIZE - smallSize) / 2.0f;

    Rectangle dest = {
        worldPos.x + offset,
        worldPos.y + offset,
        smallSize,
        smallSize
    };

    DrawTexturePro(TexturesMap[slot], source, dest, (Vector2){0,0}, 0.0f, WHITE);
}

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

    Rectangle src = GetFrame(frameX, row);
    DrawTextureRec(TexturesMap[texture], src, anim.position, tint);
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
