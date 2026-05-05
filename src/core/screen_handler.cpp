/**
 * @file screen.cpp
 * @brief Implementasi Screen & GameState Management Module
 *
 * Handle virtual screen rendering, window scaling, dan game loop entry points.
 * Semua rendering dan update logic masuk lewat fungsi-fungsi di sini.
 *
 * Arsitektur rendering:
 * - Game dirender ke RenderTexture2D virtual (1280x720)
 * - Texture virtual di-scale ke window asli sambil jaga aspect ratio
 * - Sisi yang tidak terpakai diisi black bar (letterbox)
 *
 * Urutan init yang benar:
 * InitScreen() → InitMap() → InitAll() → masuk game loop
 */

#include "screen.h"
#include "map.h"
#include "player.h"
#include "tiles.h"
#include "animation.h"
#include "enemy.h"
#include "entities.h"
#include "mapLogic.h"
#include "effects.h"
#include "debug.h"
#include "pauseMenu.h"
#include "combat.h"
#include "interaction.h"
#include <cstdio>

#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include <string>
#include <algorithm>
#include <cctype>
#include "hud.h"
#include "propsbehavior.h"

/*==============================================================================
 * External Variables & Macros
 *==============================================================================*/

GameState *gState;

extern PauseMenu pauseMenu;

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/*==============================================================================
 * Constants
 *==============================================================================*/

const float ScaleMultiplierMonitor = 0.7F;
const float ScaleMinMultiplierMonitor = 0.4F;

extern const int GameScreenWidth = 1280;
extern const int GameScreenHeight = 720;

/*==============================================================================
 * Initialization
 *==============================================================================*/

/**
 * @brief Inisialisasi semua entity dan camera di awal game
 *
 * Wajib dipanggil setelah InitMap() karena player butuh data map untuk spawn.
 * Camera langsung di-set ke posisi spawn player setelah init.
 */
void InitAll()
{
    // Bersihkan seluruh entitas sebelum inisialisasi ulang
    Entities::Clear();

    // init player — spawn point dibaca otomatis dari object layer Tiled
    PlayerInstance.Init(gState, SPAWN_OBJECT_NAME);

    // init enemy
    InitEnemy();

    // Testing spawn item
    InitItems();

    // set camera ke tengah posisi spawn player
    Vector2 spawnPos = PlayerInstance.GetPosition();
    camera.target = {spawnPos.x + (TILE_SIZE / 2.0F), spawnPos.y + (TILE_SIZE / 2.0F)};
    camera.offset = {(float)(GameScreenWidth / 2), (float)(GameScreenHeight / 2)};
    camera.rotation = 0;
    camera.zoom = 1.0F;

    // Daftarkan player ke sistem entitas agar diupdate & dirender otomatis (Index 0)
    Entities::Add(&PlayerInstance);

    // Spawn musuh dari map aktif
    SpawnEnemiesFromMap();
    SpawnObject();
}

/**
 * @brief Fungsi pembantu untuk mengubah string menjadi lowercase
 */
static std::string ToLower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c)
                   { return std::tolower(c); });
    return str;
}

// fungsi ini bingung ingin ditaruh dimana, jadi sementara disini aja
// soalnya butuh data object dari map, tapi masih berhubungan dengan entities dan enemy
void SpawnEnemiesFromMap()
{
    if (!tilesonMap)
        return;

    TraceLog(LOG_INFO, "ENEMY: Spawning enemies for current map...");

    for (auto &obj : tilesonMap->Objects)
    {
        std::string nameLower = ToLower(obj.name);
        std::string typeLower = ToLower(obj.type);

        // Cek apakah ini adalah titik spawn musuh
        bool isEnemySpawn = (nameLower.find("enemy") != std::string::npos ||
                             nameLower.find("slime") != std::string::npos ||
                             nameLower.find("skeleton") != std::string::npos ||
                             nameLower.find("wolf") != std::string::npos ||
                             obj.name == ENEMY_SPAWN_OBJECT_NAME ||
                             typeLower == "enemy_spawn");

        if (isEnemySpawn)
        {
            // 0. Cek apakah musuh ini sudah pernah dibunuh (persistence antar pindah map)
            if (Entities::IsAlreadyDead(GetCurrentMapPath(), obj.id))
            {
                TraceLog(LOG_INFO, "ENEMY: Object ID %d is already dead. Skipping spawn.", obj.id);
                continue;
            }

            // 1. Tentukan Tipe Musuh (Prioritas: Properti 'enemy_type' -> Nama Objek)
            EnemyType type = SLIME;
            bool typeFound = false;

            if (obj.properties.count("enemy_type"))
            {
                std::string typeStr = ToLower(obj.properties.at("enemy_type").getValue<std::string>());
                if (typeStr == "skeleton")
                {
                    type = SKELETON;
                    typeFound = true;
                }
                else if (typeStr == "wolf")
                {
                    type = WOLF;
                    typeFound = true;
                }
                else if (typeStr == "slime")
                {
                    type = SLIME;
                    typeFound = true;
                }
            }

            if (!typeFound)
            {
                if (nameLower.find("skeleton") != std::string::npos)
                    type = SKELETON;
                else if (nameLower.find("wolf") != std::string::npos)
                    type = WOLF;
                else if (nameLower.find("slime") != std::string::npos)
                    type = SLIME;
                else
                {
                    // Gunakan GetRandomValue (cara lama) untuk menentukan tipe musuh secara acak tanpa srand()
                    type = (EnemyType)GetRandomValue(0, 2);
                }
            }

            // 2. Tentukan Radius Patroli (Default: 128, atau dari properti 'radius')
            float radius = 128.0f;
            if (obj.properties.count("radius"))
            {
                auto prop = obj.properties.at("radius");
                if (prop.getType() == tson::Type::Int)
                    radius = (float)prop.getValue<int>();
                else if (prop.getType() == tson::Type::Float)
                    radius = prop.getValue<float>();
            }

            // 3. Spawn tepat 1 musuh di tengah objek spawn
            Vector2 spawnPos = {obj.bounds.x + obj.bounds.width / 2.0f, obj.bounds.y + obj.bounds.height / 2.0f};

            Enemy *enemy = new Enemy();
            enemy->Init(spawnPos, obj.name.c_str(), obj.id, type, radius);
            Entities::AddDynamic(enemy);

            TraceLog(LOG_INFO, "ENEMY: Created 1 enemy (Type: %d, ID: %d) from spawn point '%s'", (int)type, obj.id, obj.name.c_str());
        }
    }
}

/**
 * @brief Inisialisasi window, audio, dan render texture virtual
 *
 * Urutan init internal:
 * 1. Buat window resizable ukuran 70% monitor
 * 2. Hitung scale multiplier berdasarkan ukuran monitor
 * 3. Buat RenderTexture2D 1280x720 sebagai layar virtual
 * 4. Set FPS target ke 60
 *
 * @return GameState yang sudah diinisialisasi, siap dipakai game loop
 */
GameState InitScreen()
{
    GameState state = {{0}};

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Dungeon Game");
    InitAudioDevice();

    state.WindowScreenWidth = (int)(GetMonitorWidth(0) * ScaleMultiplierMonitor);
    state.WindowScreenHeight = (int)(GetMonitorHeight(0) * ScaleMultiplierMonitor);
    state.ScaleMultiplier = MIN(
        (float)state.WindowScreenWidth / GameScreenWidth,
        (float)state.WindowScreenHeight / GameScreenHeight);

    SetWindowSize(state.WindowScreenWidth, state.WindowScreenHeight);
    SetWindowMinSize(
        (int)(GetMonitorWidth(0) * ScaleMinMultiplierMonitor),
        (int)(GetMonitorHeight(0) * ScaleMinMultiplierMonitor));

    state.Dungeon = LoadRenderTexture(GameScreenWidth, GameScreenHeight);
    SetTextureFilter(state.Dungeon.texture, TEXTURE_FILTER_BILINEAR);

    const int FPS = 60;
    SetTargetFPS(FPS);

    state.currentScreen = MAIN_MENU;
    state.showFPS = false;
    state.isSwitchingMap = false;
    state.isGoingBack = false;
    state.pendingMapPath.clear();
    state.pendingDoorName.clear();

    return state;
}

/*==============================================================================
 * Update
 *==============================================================================*/

/**
 * @brief Update ukuran window dan scale multiplier tiap frame
 */
void UpdateGame(GameState *state)
{
    state->WindowScreenWidth = GetScreenWidth();
    state->WindowScreenHeight = GetScreenHeight();
    state->ScaleMultiplier = MIN(
        (float)state->WindowScreenWidth / GameScreenWidth,
        (float)state->WindowScreenHeight / GameScreenHeight);
}

/**
 * @brief Entry point update logic game — dipanggil sekali per frame
 *
 * Menggunakan Entities::Update() sebagai entry point utama update semua entity.
 * Player.Update() dan Enemy.Update() dipanggil otomatis dari situ.
 */
void UpdateLogicAll()
{
    // Update semua entity (Player + semua Enemy) via Entities registry
    Entities::Update();

    // Handle pending map transitions dari Interaction namespace
    Interaction::ExecutePendingTransitions(PlayerInstance);

    // Update Effects (Popups, Logs, etc)
    Effects::Update(GetFrameTime());
    spikeManager.Update(GetFrameTime(), PlayerInstance.GetHitbox(), &PlayerInstance);
    bombManager.Update(GetFrameTime(), PlayerInstance.GetHitbox(), &PlayerInstance);

    // Update item magnet/pickup
    Vector2 center = PlayerInstance.GetCenter();
    Rectangle pHitbox = {
        center.x - PlayerInstance.GetHitboxWidth() / 2,
        center.y - PlayerInstance.GetHitboxHeight() / 2,
        PlayerInstance.GetHitboxWidth(),
        PlayerInstance.GetHitboxHeight()};

    itemRender.Update(itemData.activeItems, center, pHitbox, PlayerInstance.GetMagnetRadius(), PlayerInstance.GetItemSpeed());

    // Pickup scan setelah magnet update
    for (auto &item : itemData.activeItems)
    {
        if (item.isPickedUp && !item.isAdded)
        {
            TraceLog(LOG_INFO, "PICKUP: trying to add definitionId=%d", item.definitionId);
            if (Inventory::AddToInventory(PlayerInstance, item))
            {
                TraceLog(LOG_INFO, "PICKUP: added to inventory");
                item.isAdded = true;
            }
            else
            {
                TraceLog(LOG_INFO, "PICKUP: inventory full");
                item.isPickedUp = false; // balik ke world
            }
        }
    }
}

/*==============================================================================
 * Rendering
 *==============================================================================*/

/**
 * @brief Entry point render — semua yang terlihat di layar lewat sini
 *
 * Urutan layer render (urutan penting, jangan diubah sembarangan):
 * 1. Map — tile-based, tidak kena transform camera
 * 2. Entities, Items, Effects & world overlay — dalam BeginMode2D (world space)
 * 3. Debug panels — screen space, di luar BeginMode2D
 * 4. UI overlay — HUD, pause menu, dll
 */
void DrawRenderTexture(GameState *state)
{
    BeginTextureMode(state->Dungeon);
    ClearBackground(BLACK);

    // layer 1: tile map
    RenderMap();

    // layer 2: entities, items, effects & world overlay (world space)
    BeginMode2D(camera);
    RenderTileProps();
    itemRender.RenderAll(itemData.activeItems);
    Entities::Render();
    Effects::Draw();
    DebugInstance.DrawWorldOverlay();
    EndMode2D();

    // layer 3: debug panels (screen space)
    DebugInstance.Toggle();
    DebugInstance.Draw();

    // layer 4: UI overlay
    DrawUIOverlay(state);

    EndTextureMode();
}

/**
 * @brief Render UI elements ke virtual screen
 */
void DrawUIOverlay(GameState *state)
{
    DrawPlayerHUD();

    // 2. FPS Counter (if enabled)
    if (state->showFPS)
    {
        int fps = GetFPS();
        char fpsText[16];
        snprintf(fpsText, sizeof(fpsText), "FPS: %d", fps);
        DrawText(fpsText, 10, 10, 20, GREEN);
    }

    // 3. Menus
    if (pauseMenu.IsActive())
    {
        Vector2 mousePos = GetVirtualMousePosition(state);
        pauseMenu.Draw(mousePos);
    }
}

/**
 * @brief Scale dan render layar virtual ke window asli
 */
void DrawRenderWindows(GameState *state)
{
    float offsetX = (state->WindowScreenWidth - ((float)GameScreenWidth * state->ScaleMultiplier)) * 0.5F;
    float offsetY = (state->WindowScreenHeight - ((float)GameScreenHeight * state->ScaleMultiplier)) * 0.5F;

    BeginDrawing();
    ClearBackground(BLACK);

    DrawTexturePro(
        state->Dungeon.texture,
        (Rectangle){0, 0, (float)GameScreenWidth, -(float)GameScreenHeight},
        (Rectangle){offsetX, offsetY, (float)GameScreenWidth * state->ScaleMultiplier, (float)GameScreenHeight * state->ScaleMultiplier},
        (Vector2){0, 0},
        0.0F,
        WHITE);

    EndDrawing();
}

/*==============================================================================
 * Utilities
 *==============================================================================*/

/**
 * @brief Konversi posisi mouse dari window space ke virtual screen space
 */
Vector2 GetVirtualMousePosition(GameState *state)
{
    Vector2 mouse = GetMousePosition();
    Vector2 virtualMouse = {0, 0};

    virtualMouse.x = (mouse.x - ((state->WindowScreenWidth - (GameScreenWidth * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;
    virtualMouse.y = (mouse.y - ((state->WindowScreenHeight - (GameScreenHeight * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;

    return Vector2Clamp(virtualMouse, (Vector2){0, 0}, (Vector2){(float)GameScreenWidth, (float)GameScreenHeight});
}

/*==============================================================================
 * Cleanup
 *==============================================================================*/

/**
 * @brief Bersihin semua resource sebelum game ditutup
 */
void GameShutDown(GameState *state)
{
    for (int i = 0; i < MAX_TEXTURES; i++)
        UnloadTexture(TexturesMap[i]);

    Entities::Shutdown();
    UnloadMap();
    UnloadRenderTexture(state->Dungeon);

    CloseAudioDevice();
    CloseWindow();
}

/*==============================================================================
 * Window & Video Settings Functions
 *==============================================================================*/

void ToggleFullscreenMode(void)
{
    if (IsWindowFullscreen())
    {
        ToggleFullscreen();
    }
    else
    {
        ToggleFullscreen();
    }
}

void SetResolution(int width, int height)
{
    SetWindowSize(width, height);
}

Rectangle GetCurrentResolution(void)
{
    Rectangle res = {0};
    res.width = static_cast<float>(GetScreenWidth());
    res.height = static_cast<float>(GetScreenHeight());
    return res;
}

Rectangle GetMonitorResolution(void)
{
    Rectangle res = {0};
    res.width = static_cast<float>(GetMonitorWidth(0));
    res.height = static_cast<float>(GetMonitorHeight(0));
    return res;
}

bool IsFullscreen(void)
{
    return IsWindowFullscreen();
}
