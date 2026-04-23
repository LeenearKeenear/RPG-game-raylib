#include "../include/animation.h"

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
        [IDLE] = {
            [LEFT]  = {0, 0, 2, 0.5f, true, {0, 1}, 2},
            [RIGHT] = {0, 0, 2, 0.5f, true, {0, 1}, 2},
            [DOWN]  = {0, 0, 2, 0.5f, true, {0, 1}, 2},
            [UP]    = {0, 0, 2, 0.5f, true, {0, 1}, 2}
        },
        [WALK] = {
            [LEFT]  = {0, 0, 2, 0.5f, true, {0, 1}, 2},
            [RIGHT] = {0, 0, 2, 0.5f, true, {0, 1}, 2},
            [DOWN]  = {0, 0, 2, 0.5f, true, {0, 1}, 2},
            [UP]    = {0, 0, 2, 0.5f, true, {0, 1}, 2}
        },
        [ATTACK] = {
            [LEFT]  = {0, 0, 2, 0.5f, true, {0, 1}, 2},
            [RIGHT] = {0, 0, 2, 0.5f, true, {0, 1}, 2},
            [DOWN]  = {0, 0, 2, 0.5f, true, {0, 1}, 2},
            [UP]    = {0, 0, 2, 0.5f, true, {0, 1}, 2}
        },
        [DEAD] = {
            [LEFT]  = {0, 0, 2, 0.5f, true, {0, 1}, 2},
            [RIGHT] = {0, 0, 2, 0.5f, true, {0, 1}, 2},
            [DOWN]  = {0, 0, 2, 0.5f, true, {0, 1}, 2},
            [UP]    = {0, 0, 2, 0.5f, true, {0, 1}, 2}
        }
    }
};

const AnimationSet SkeletonAnimationSet = {
    .configs = {
        [IDLE] = {
            [LEFT]  = {1, 0, 2, 0.5f, true, {0, 1}, 2},
            [RIGHT] = {1, 0, 2, 0.5f, true, {0, 1}, 2},
            [DOWN]  = {1, 0, 2, 0.5f, true, {0, 1}, 2},
            [UP]    = {1, 0, 2, 0.5f, true, {0, 1}, 2}
        },
        [WALK] = {
            [LEFT]  = {1, 0, 2, 0.5f, true, {0, 1}, 2},
            [RIGHT] = {1, 0, 2, 0.5f, true, {0, 1}, 2},
            [DOWN]  = {1, 0, 2, 0.5f, true, {0, 1}, 2},
            [UP]    = {1, 0, 2, 0.5f, true, {0, 1}, 2}
        },
        [ATTACK] = {
            [LEFT]  = {1, 0, 2, 0.5f, true, {0, 1}, 2},
            [RIGHT] = {1, 0, 2, 0.5f, true, {0, 1}, 2},
            [DOWN]  = {1, 0, 2, 0.5f, true, {0, 1}, 2},
            [UP]    = {1, 0, 2, 0.5f, true, {0, 1}, 2}
        },
        [DEAD] = {
            [LEFT]  = {1, 0, 2, 0.5f, true, {0, 1}, 2},
            [RIGHT] = {1, 0, 2, 0.5f, true, {0, 1}, 2},
            [DOWN]  = {1, 0, 2, 0.5f, true, {0, 1}, 2},
            [UP]    = {1, 0, 2, 0.5f, true, {0, 1}, 2}
        }
    }
};

const AnimationSet WolfAnimationSet = {
    .configs = {
        [IDLE] = {
            [LEFT]  = {2, 0, 2, 0.5f, true, {0, 1}, 2},
            [RIGHT] = {2, 0, 2, 0.5f, true, {0, 1}, 2},
            [DOWN]  = {2, 0, 2, 0.5f, true, {0, 1}, 2},
            [UP]    = {2, 0, 2, 0.5f, true, {0, 1}, 2}
        },
        [WALK] = {
            [LEFT]  = {2, 0, 2, 0.5f, true, {0, 1}, 2},
            [RIGHT] = {2, 0, 2, 0.5f, true, {0, 1}, 2},
            [DOWN]  = {2, 0, 2, 0.5f, true, {0, 1}, 2},
            [UP]    = {2, 0, 2, 0.5f, true, {0, 1}, 2}
        },
        [ATTACK] = {
            [LEFT]  = {2, 0, 2, 0.5f, true, {0, 1}, 2},
            [RIGHT] = {2, 0, 2, 0.5f, true, {0, 1}, 2},
            [DOWN]  = {2, 0, 2, 0.5f, true, {0, 1}, 2},
            [UP]    = {2, 0, 2, 0.5f, true, {0, 1}, 2}
        },
        [DEAD] = {
            [LEFT]  = {2, 0, 2, 0.5f, true, {0, 1}, 2},
            [RIGHT] = {2, 0, 2, 0.5f, true, {0, 1}, 2},
            [DOWN]  = {2, 0, 2, 0.5f, true, {0, 1}, 2},
            [UP]    = {2, 0, 2, 0.5f, true, {0, 1}, 2}
        }
    }
};

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
                    if (anim.set) PlayAnimation(anim, IDLE, anim.direction, *anim.set);
                }
            }
        }
        
        anim.currentFrame = anim.currentConfig->pattern[anim.walkFrameIndex];
        
        if (anim.state == ATTACK && anim.walkFrameIndex == 1)
        {
            TraceLog(LOG_INFO, "HIT!");
        }
    }
}

void DrawAnimation(const Animation &anim, TextureAsset texture)
{
    if (!anim.currentConfig) return;

    int frameX = anim.currentConfig->startFrame + anim.currentFrame;
    int row = anim.currentConfig->row;

    Rectangle src = GetFrame(frameX, row);
    DrawTextureRec(TexturesMap[texture], src, anim.position, WHITE);
}

void PlayAnimation(Animation &anim, State newState, Direction newDir, const AnimationSet &set)
{
    if (anim.state == newState && anim.direction == newDir && anim.currentConfig) return;

    anim.state = newState;
    anim.direction = newDir;
    anim.set = &set;
    anim.currentConfig = &set.configs[newState][newDir];
    
    anim.timer = 0;
    anim.walkFrameIndex = 0;
    anim.currentFrame = anim.currentConfig->pattern[0];
}
