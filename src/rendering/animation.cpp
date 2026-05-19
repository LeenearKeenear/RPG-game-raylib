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
static std::unordered_map<std::string, Frame> loadedFrames;

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
                    Frame f;
                    f.texture = ResolveTextureSlot(val.at("texture").get<std::string>());
                    f.positionX = val.at("column").get<int>();
                    f.positionY = val.at("row").get<int>();
                    f.width = val.contains("width") ? val.at("width").get<int>() : 1;
                    f.height = val.contains("height") ? val.at("height").get<int>() : 1;
                    loadedFrames[key] = f;
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
                    Frame f;
                    f.texture = ResolveTextureSlot(val.at("texture").get<std::string>());
                    f.positionX = val.at("column").get<int>();
                    f.positionY = val.at("row").get<int>();
                    f.width = val.contains("width") ? val.at("width").get<int>() : 1;
                    f.height = val.contains("height") ? val.at("height").get<int>() : 1;
                    loadedFrames[key] = f;
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

void InitTextures()
{
    LoadFrameTexture(TILESET_MAP_1, "assets/textures/tiles.png");
    LoadFrameTexture(TILESET_MAP_2, "assets/textures/test.png");
    LoadFrameTexture(TILESET_PROPS, "assets/textures/props.png");
    LoadFrameTexture(TILESET_ITEMS, "assets/textures/items.png");
    LoadFrameTexture(SPRITESHEET_KNIGHT, "assets/textures/knight (1).png");
    LoadFrameTexture(SPRITESHEET_ENEMIES, "assets/textures/enemies.png");
    LoadFramesFromJSON();
    LoadAnimationsFromJSON();
}

void CloseTextures()
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

const Frame &GetFrame(const std::string &id)
{
    auto it = loadedFrames.find(id);
    if (it != loadedFrames.end())
    {
        return it->second;
    }
    
    TraceLog(LOG_WARNING, "ANIMATION: Frame not found: %s", id.c_str());
    static const Frame fallback = { TILESET_MAP_1, 0, 0, 1, 1 };
    return fallback;
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

void DrawFrame(const std::string &id, Display display)
{
    DrawFrame(GetFrame(id), display);
}

/*
====================
Sprites Animation
====================
*/

std::unordered_map<std::string, AnimationSet> loadedAnimationSets;

static State ResolveState(const std::string &str)
{
    static const std::unordered_map<std::string, State> mapping = {
        {"idle", IDLE},
        {"walk", WALK},
        {"attack", ATTACK},
        {"dead", DEAD}
    };
    auto it = mapping.find(str);
    if (it != mapping.end()) return it->second;
    return IDLE;
}

static Direction ResolveDirection(const std::string &str)
{
    static const std::unordered_map<std::string, Direction> mapping = {
        {"left", LEFT},
        {"right", RIGHT},
        {"down", DOWN},
        {"up", UP}
    };
    auto it = mapping.find(str);
    if (it != mapping.end()) return it->second;
    return RIGHT;
}

void LoadAnimationsFromJSON()
{
    std::ifstream file("assets/data/animations.json");
    if (!file.is_open())
    {
        TraceLog(LOG_ERROR, "ANIMATION: Gagal membuka assets/data/animations.json");
        return;
    }

    try
    {
        json root = json::parse(file);
        for (auto &[entityKey, entityVal] : root.items())
        {
            AnimationSet set;
            for (int s = 0; s < 4; s++) {
                for (int d = 0; d < 4; d++) {
                    set.configs[s][d].speed = 0.5f;
                    set.configs[s][d].loop = true;
                }
            }

            for (auto &[stateKey, stateVal] : entityVal.items())
            {
                State state = ResolveState(stateKey);
                
                for (auto &[dirKey, dirVal] : stateVal.items())
                {
                    Direction dir = ResolveDirection(dirKey);
                    
                    AnimationConfig config;
                    config.speed = dirVal.at("frameDuration").get<float>();
                    config.loop = dirVal.at("loop").get<bool>();
                    
                    if (dirVal.contains("frames"))
                    {
                        for (auto &frame : dirVal.at("frames"))
                        {
                            config.frameIds.push_back(frame.get<std::string>());
                        }
                    }
                    
                    set.configs[state][dir] = config;
                }
                
                for (int d = 0; d < 4; d++)
                {
                    if (set.configs[state][d].frameIds.empty())
                    {
                        if (!set.configs[state][LEFT].frameIds.empty())
                        {
                            set.configs[state][d] = set.configs[state][LEFT];
                        }
                        else if (!set.configs[state][RIGHT].frameIds.empty())
                        {
                            set.configs[state][d] = set.configs[state][RIGHT];
                        }
                    }
                }
            }
            loadedAnimationSets[entityKey] = set;
        }
        TraceLog(LOG_INFO, "ANIMATION: Berhasil memuat %d animation sets dari JSON", (int)loadedAnimationSets.size());
    }
    catch (const std::exception &e)
    {
        TraceLog(LOG_ERROR, "ANIMATION: Gagal parse animations.json: %s", e.what());
    }
}

void PlayAnimation(Animation &anim, State newState, Direction newDir)
{
    if (!anim.animSet) return;

    Direction resolvedDir = newDir;
    
    if (anim.animSet->configs[newState][resolvedDir].frameIds.empty())
    {
        if (resolvedDir == UP || resolvedDir == DOWN)
        {
            if (anim.direction == LEFT || anim.direction == RIGHT)
            {
                resolvedDir = anim.direction;
            }
            else
            {
                resolvedDir = RIGHT;
            }
        }
        
        if (anim.animSet->configs[newState][resolvedDir].frameIds.empty())
        {
            if (!anim.animSet->configs[newState][LEFT].frameIds.empty()) resolvedDir = LEFT;
            else if (!anim.animSet->configs[newState][RIGHT].frameIds.empty()) resolvedDir = RIGHT;
        }
    }

    if (anim.state == newState && anim.direction == resolvedDir && anim.currentConfig) return;

    anim.state = newState;
    anim.direction = resolvedDir;
    anim.currentConfig = &anim.animSet->configs[newState][resolvedDir];
    
    anim.timer = 0.0f;
    anim.currentFrameIndex = 0;
}

void UpdateAnimation(Animation &anim, float dt)
{
    if (!anim.currentConfig || anim.currentConfig->frameIds.empty()) return;

    anim.timer += dt;
    if (anim.timer >= anim.currentConfig->speed)
    {
        anim.timer = 0.0f;
        anim.currentFrameIndex++;

        if (anim.currentFrameIndex >= (int)anim.currentConfig->frameIds.size())
        {
            if (anim.currentConfig->loop)
            {
                anim.currentFrameIndex = 0;
            }
            else
            {
                anim.currentFrameIndex = (int)anim.currentConfig->frameIds.size() - 1;
                
                if (anim.state == ATTACK)
                {
                    anim.isAttacking = false;
                    PlayAnimation(anim, IDLE, anim.direction);
                }
            }
        }
    }
}

void DrawAnimation(const Animation &anim, Color tint)
{
    if (!anim.currentConfig || anim.currentConfig->frameIds.empty()) return;

    int index = anim.currentFrameIndex;
    if (index < 0 || index >= (int)anim.currentConfig->frameIds.size())
    {
        index = 0;
    }

    const std::string &frameId = anim.currentConfig->frameIds[index];
    Display display = { anim.position, FRAME_SIZE, {0,0}, {0,0}, 0.0f, tint };
    DrawFrame(frameId, display);
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
