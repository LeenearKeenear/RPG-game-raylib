#pragma once
#include "../lib/raylib/include/raylib.h"
#include <string>
#include <vector>
#include <unordered_map>

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
void InitTextures();
void CloseTextures();
const Frame &GetFrame(const std::string &id);
void DrawFrame(Frame frame, Display display);
void DrawFrame(const std::string &id, Display display);

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

struct AnimationConfig
{
    std::vector<std::string> frameIds;
    float speed;
    bool loop;
};

struct AnimationSet
{
    AnimationConfig configs[4][4];
};

struct Animation
{
    Vector2 position;
    State state;
    Direction direction;
    float timer;
    int currentFrameIndex;
    bool isAttacking;
    bool isDead;
    const AnimationConfig *currentConfig;
    const AnimationSet *animSet;
};

void LoadAnimationsFromJSON();
void PlayAnimation(Animation &anim, State newState, Direction newDir);
void UpdateAnimation(Animation &anim, float dt);
void DrawAnimation(const Animation &anim, Color tint = WHITE);

extern std::unordered_map<std::string, AnimationSet> loadedAnimationSets;

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
