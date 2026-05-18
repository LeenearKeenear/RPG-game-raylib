#include "animation.h"
#include "../lib/raylib/include/raymath.h"
#include "../lib/json/include/nlohmann/json.hpp"
#include <fstream>
#include <unordered_map>
#include <string>

using json = nlohmann::json;

/*
====================
Frames Management
====================
*/

Texture2D textures[MAX_TEXTURES];
Frame tileFrames[TILE_ID_COUNT];
Frame spriteFrames[SPRITE_ID_COUNT];

static TextureSlot ResolveTextureSlot(const std::string &str)
{
    static const std::unordered_map<std::string, TextureSlot> mapping = {
        {"TILESET_MAP_1", TILESET_MAP_1},
        {"TILESET_MAP_2", TILESET_MAP_2},
        {"TILESET_PROPS", TILESET_PROPS},
        {"TILESET_ITEMS", TILESET_ITEMS},
        {"SPRITESHEET_KNIGHT", SPRITESHEET_KNIGHT},
        {"SPRITESHEET_ENEMIES", SPRITESHEET_ENEMIES}
    };
    auto it = mapping.find(str);
    if (it != mapping.end()) return it->second;
    return TILESET_MAP_1;
}

static TileId ResolveTileId(const std::string &str)
{
    static const std::unordered_map<std::string, TileId> mapping = {
        {"spikeInactive", SPIKE_INACTIVE},
        {"spikeActive", SPIKE_ACTIVE},
        {"bomb", BOMB},
        {"chestClosed", CHEST_CLOSED},
        {"chestOpen", CHEST_OPEN},
        {"crate", CRATE},
        {"sword1", SWORD_1},
        {"sword2", SWORD_2},
        {"bow", BOW},
        {"healthPotion", POTION_HEALTH},
        {"staminaPotion", POTION_STAMINA}
    };
    auto it = mapping.find(str);
    if (it != mapping.end()) return it->second;
    return TILE_ID_COUNT;
}

static SpriteId ResolveSpriteId(const std::string &str)
{
    static const std::unordered_map<std::string, SpriteId> mapping = {
        {"knightIdleRight1", KNIGHT_IDLE_RIGHT_1},
        {"knightIdleRight2", KNIGHT_IDLE_RIGHT_2},
        {"knightIdleLeft1", KNIGHT_IDLE_LEFT_1},
        {"knightIdleLeft2", KNIGHT_IDLE_LEFT_2},
        {"knightWalkRight1", KNIGHT_WALK_RIGHT_1},
        {"knightWalkRight2", KNIGHT_WALK_RIGHT_2},
        {"knightWalkRight3", KNIGHT_WALK_RIGHT_3},
        {"knightWalkRight4", KNIGHT_WALK_RIGHT_4},
        {"knightWalkRight5", KNIGHT_WALK_RIGHT_5},
        {"knightWalkRight6", KNIGHT_WALK_RIGHT_6},
        {"knightWalkRight7", KNIGHT_WALK_RIGHT_7},
        {"knightWalkLeft1", KNIGHT_WALK_LEFT_1},
        {"knightWalkLeft2", KNIGHT_WALK_LEFT_2},
        {"knightWalkLeft3", KNIGHT_WALK_LEFT_3},
        {"knightWalkLeft4", KNIGHT_WALK_LEFT_4},
        {"knightWalkLeft5", KNIGHT_WALK_LEFT_5},
        {"knightWalkLeft6", KNIGHT_WALK_LEFT_6},
        {"knightWalkLeft7", KNIGHT_WALK_LEFT_7},
        {"knightDeadRight", KNIGHT_DEAD_RIGHT},
        {"knightDeadLeft", KNIGHT_DEAD_LEFT},
        {"slimeIdleLeft1", SLIME_IDLE_LEFT_1},
        {"slimeIdleLeft2", SLIME_IDLE_LEFT_2},
        {"slimeDeadLeft", SLIME_DEAD_LEFT},
        {"skeletonIdleLeft1", SKELETON_IDLE_LEFT_1},
        {"skeletonIdleLeft2", SKELETON_IDLE_LEFT_2},
        {"skeletonDeadLeft", SKELETON_DEAD_LEFT},
        {"wolfIdleLeft1", WOLF_IDLE_LEFT_1},
        {"wolfIdleLeft2", WOLF_IDLE_LEFT_2},
        {"wolfDeadLeft", WOLF_DEAD_LEFT}
    };
    auto it = mapping.find(str);
    if (it != mapping.end()) return it->second;
    return SPRITE_ID_COUNT;
}

static void LoadFramesFromJSON()
{
    {
        std::ifstream file("assets/data/tiles.json");
        if (!file.is_open())
        {
            TraceLog(LOG_ERROR, "ANIMATION: Cannot open assets/data/tiles.json");
        }
        else
        {
            try
            {
                json root = json::parse(file);
                for (auto &[key, val] : root.items())
                {
                    TileId id = ResolveTileId(key);
                    if (id != TILE_ID_COUNT)
                    {
                        Frame f;
                        f.texture = ResolveTextureSlot(val.at("texture").get<std::string>());
                        f.positionX = val.at("column").get<int>();
                        f.positionY = val.at("row").get<int>();
                        f.width = val.contains("width") ? val.at("width").get<int>() : 1;
                        f.height = val.contains("height") ? val.at("height").get<int>() : 1;
                        tileFrames[id] = f;
                    }
                }
                TraceLog(LOG_INFO, "ANIMATION: Successfully loaded tiles.json");
            }
            catch (const std::exception &e)
            {
                TraceLog(LOG_ERROR, "ANIMATION: Error parsing tiles.json: %s", e.what());
            }
        }
    }

    {
        std::ifstream file("assets/data/sprites.json");
        if (!file.is_open())
        {
            TraceLog(LOG_ERROR, "ANIMATION: Cannot open assets/data/sprites.json");
        }
        else
        {
            try
            {
                json root = json::parse(file);
                for (auto &[key, val] : root.items())
                {
                    SpriteId id = ResolveSpriteId(key);
                    if (id != SPRITE_ID_COUNT)
                    {
                        Frame f;
                        f.texture = ResolveTextureSlot(val.at("texture").get<std::string>());
                        f.positionX = val.at("column").get<int>();
                        f.positionY = val.at("row").get<int>();
                        f.width = val.contains("width") ? val.at("width").get<int>() : 1;
                        f.height = val.contains("height") ? val.at("height").get<int>() : 1;
                        spriteFrames[id] = f;
                    }
                }
                TraceLog(LOG_INFO, "ANIMATION: Successfully loaded sprites.json");
            }
            catch (const std::exception &e)
            {
                TraceLog(LOG_ERROR, "ANIMATION: Error parsing sprites.json: %s", e.what());
            }
        }
    }
}

void LoadFrameTexture(TextureSlot slot, const char *path)
{
    Image img = LoadImage(path);
    textures[slot] = LoadTextureFromImage(img);
    UnloadImage(img);
}

const Frame &GetFrame(TileId id)
{
    if (id < 0 || id >= TILE_ID_COUNT)
    {
        static const Frame fallback = { TILESET_MAP_1, 0, 0, 1, 1 };
        return fallback;
    }
    return tileFrames[id];
}

const Frame &GetFrame(SpriteId id)
{
    if (id < 0 || id >= SPRITE_ID_COUNT)
    {
        static const Frame fallback = { SPRITESHEET_KNIGHT, 0, 0, 1, 1 };
        return fallback;
    }
    return spriteFrames[id];
}

void DrawFrame(Frame frame, Display display)
{
    Rectangle src = {
        (float)(frame.positionX * (FRAME_SIZE + FRAME_GAP)),
        (float)(frame.positionY * (FRAME_SIZE + FRAME_GAP)),
        (float)(frame.width * FRAME_SIZE),
        (float)(frame.height * FRAME_SIZE)
    };
    Rectangle dest = {
        display.position.x + display.offset.x,
        display.position.y + display.offset.y,
        (float)(frame.width * display.size),
        (float)(frame.height * display.size)
    };
    DrawTexturePro(textures[frame.texture], src, dest, display.origin, display.rotation, display.tint);
}

void InitAnimationSystem()
{
    LoadFrameTexture(TILESET_MAP_1, "assets/textures/tiles.png");
    LoadFrameTexture(TILESET_MAP_2, "assets/textures/test.png");
    LoadFrameTexture(TILESET_PROPS, "assets/textures/props.png");
    LoadFrameTexture(TILESET_ITEMS, "assets/textures/items.png");
    LoadFrameTexture(SPRITESHEET_KNIGHT, "assets/textures/knight (1).png");
    LoadFrameTexture(SPRITESHEET_ENEMIES, "assets/textures/enemies.png");

    LoadFramesFromJSON();
}

void CloseAnimationSystem()
{
    for (int i = 0; i < MAX_TEXTURES; i++)
    {
        if (textures[i].id != 0)
        {
            UnloadTexture(textures[i]);
            textures[i] = {0};
        }
    }
}

/*
====================
Sprites Animation
====================
*/

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

void DrawAnimation(const Animation &anim, TextureSlot texture, Color tint)
{
    if (!anim.currentConfig) return;

    int frameX = anim.currentConfig->startFrame + anim.currentFrame;
    int row = anim.currentConfig->row;

    Frame customFrame = { texture, frameX, row, 1, 1 };
    Display display = { anim.position, FRAME_SIZE, {0,0}, {0,0}, 0.0f, tint };
    DrawFrame(customFrame, display);
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

/*
====================
Procedural Animation
====================
*/

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
    pos = Vector2Add(pos, Vector2Scale(vel, dt * 60.0f));
    
    vel.y += gravity;
    
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
    float easedProgress = sinf(progress * PI / 2.0f);
    return startAngle + (easedProgress * sweepAngle);
}
