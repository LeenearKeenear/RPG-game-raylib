#pragma once

#include "raylib.h"
#include "tiles.h"

enum State
{
    IDLE,
    WALK,
    ATTACK,
    DEAD
};

enum Direction
{
    LEFT,
    RIGHT,
    DOWN,
    UP
};

struct AnimationConfig
{
    int row;
    int startFrame;
    int frameCount;
    float speed;
    bool loop;
    int pattern[8];
    int patternCount;
};

struct Animation
{
    Vector2 position;
    State state;
    Direction direction;
    int currentFrame;
    float timer;
    int walkFrameIndex;
    bool isAttacking;
    bool isDead;
    const AnimationConfig *currentConfig;
};

struct AnimationSet
{
    AnimationConfig configs[4][4]; // [State][Direction]
};

void UpdateAnimation(Animation &anim, float dt);
void DrawAnimation(const Animation &anim, TextureAsset texture);
void PlayAnimation(Animation &anim, State newState, Direction newDir, const AnimationSet &set);

extern const AnimationSet PlayerAnimationSet;