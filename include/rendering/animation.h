#pragma once
#include "../lib/raylib/include/raylib.h"

/*
====================
Frames Management
====================
*/

constexpr int FRAME_SIZE = 32;
constexpr int FRAME_GAP = 4;
constexpr int MAX_TEXTURES = 6;

extern Texture2D textures[MAX_TEXTURES];

enum TextureSlot
{
    TILESET_MAP_1,
    TILESET_MAP_2,
    TILESET_PROPS,
    TILESET_ITEMS,
    SPRITESHEET_KNIGHT,
    SPRITESHEET_ENEMIES
};

enum TileID
{
    SPIKE_INACTIVE,
    SPIKE_ACTIVE,
    BOMB,
    CHEST_CLOSED,
    CHEST_OPEN,
    SWORD_1,
    SWORD_2,
    BOW,
    POTION_HEALTH,
    POTION_STAMINA,
    TILE_ID_COUNT
};

enum SpriteID
{
    KNIGHT_IDLE_RIGHT_1,
    KNIGHT_IDLE_RIGHT_2,
    KNIGHT_IDLE_LEFT_1,
    KNIGHT_IDLE_LEFT_2,
    KNIGHT_WALK_RIGHT_1,
    KNIGHT_WALK_RIGHT_2,
    KNIGHT_WALK_RIGHT_3,
    KNIGHT_WALK_RIGHT_4,
    KNIGHT_WALK_RIGHT_5,
    KNIGHT_WALK_RIGHT_6,
    KNIGHT_WALK_RIGHT_7,
    KNIGHT_WALK_LEFT_1,
    KNIGHT_WALK_LEFT_2,
    KNIGHT_WALK_LEFT_3,
    KNIGHT_WALK_LEFT_4,
    KNIGHT_WALK_LEFT_5,
    KNIGHT_WALK_LEFT_6,
    KNIGHT_WALK_LEFT_7,
    KNIGHT_DEAD_RIGHT,
    KNIGHT_DEAD_LEFT,
    SLIME_IDLE_LEFT_1,
    SLIME_IDLE_LEFT_2,
    SLIME_DEAD_LEFT,
    SKELETON_IDLE_LEFT_1,
    SKELETON_IDLE_LEFT_2,
    SKELETON_DEAD_LEFT,
    WOLF_IDLE_LEFT_1,
    WOLF_IDLE_LEFT_2,
    WOLF_DEAD_LEFT,
    SPRITE_ID_COUNT
};

struct Frame
{
    TextureSlot texture;
    int positionX;
    int positionY;
    int width = 1;
    int height = 1;
};

struct Display
{
    Vector2 position;
    int size = FRAME_SIZE;
    Vector2 offset = {0, 0};
    Vector2 origin = {0, 0};
    float rotation = 0.0f;
    Color tint = WHITE;
};

void LoadFrameTexture(TextureSlot slot, const char *path);
const Frame &GetFrame(TileID id);
const Frame &GetFrame(SpriteID id);
void DrawFrame(Frame frame, Display display);
void InitAnimationSystem();
void CloseAnimationSystem();

template <typename T>
void DrawFrame(T id, Display display)
{
    DrawFrame(GetFrame(id), display);
}

/*
====================
Sprites Animation
====================
*/

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

struct AnimationSet;

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
    const AnimationSet *animSet;
};

struct AnimationSet
{
    AnimationConfig configs[4][4];
};

void UpdateAnimation(Animation &anim, float dt);

void DrawAnimation(const Animation &anim, TextureSlot texture, Color tint = WHITE);

void PlayAnimation(Animation &anim, State newState, Direction newDir, const AnimationSet &set);

extern const AnimationSet PlayerAnimationSet;
extern const AnimationSet SlimeAnimationSet;
extern const AnimationSet SkeletonAnimationSet;
extern const AnimationSet WolfAnimationSet;

/*
====================
Procedural Animation
====================
*/

namespace AnimEffects
{
    float CalculateFadeOut(float timer, float duration);

    float CalculateFloatOffset(float currentOffset, float speed, float dt);

    bool ShouldBlink(float timer, float frequency);

    void ApplyPhysics(Vector2& pos, Vector2& vel, float gravity, float friction, float dt);

    Vector2 LerpTowards(Vector2 current, Vector2 target, float speed, float dt);

    float CalculateThrustOffset(float progress, float maxOffset);

    float CalculateSlashRotation(float progress, float startAngle, float sweepAngle);
}

void UpdatePlayerAttack(Animation &p);
