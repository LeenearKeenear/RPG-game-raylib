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

#include "../include/screen.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../include/tiles.h"
#include "../include/animation.h"
#include "../include/entities.h"
#include "../include/enemy.h"
#include "../include/debug.h"
#include "../include/mapLogic.h"
#include "../include/pauseMenu.h"
#include "../include/combat.h"

#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include <string>
#include <algorithm>
#include <cctype>

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

    // spawn enemies dari data map
    Entities::SpawnEnemies();

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
}

/**
 * @brief Fungsi pembantu untuk mengubah string menjadi lowercase
 */
static std::string ToLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
    return str;
}

// fungsi ini bingung ingin ditaruh dimana, jadi sementara disini aja
// soalnya butuh data object dari map, tapi masih berhubungan dengan entities dan enemy
void SpawnEnemiesFromMap()
{
    if (!tilesonMap) return;

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
            // 1. Tentukan Tipe Musuh (Prioritas: Properti 'enemy_type' -> Nama Objek)
            EnemyType type = SLIME;
            bool typeFound = false;

            if (obj.properties.count("enemy_type")) {
                std::string typeStr = ToLower(obj.properties.at("enemy_type").getValue<std::string>());
                if (typeStr == "skeleton") { type = SKELETON; typeFound = true; }
                else if (typeStr == "wolf") { type = WOLF; typeFound = true; }
                else if (typeStr == "slime") { type = SLIME; typeFound = true; }
            }
            
            if (!typeFound) {
                if (nameLower.find("skeleton") != std::string::npos) type = SKELETON;
                else if (nameLower.find("wolf") != std::string::npos) type = WOLF;
                else if (nameLower.find("slime") != std::string::npos) type = SLIME;
                else {
                    // Randomize between SLIME (0), SKELETON (1), and WOLF (2)
                    int randVal = GetRandomValue(0, 2);
                    type = (EnemyType)randVal;
                }
            }

            // 2. Tentukan Radius Patroli (Default: 128, atau dari properti 'radius')
            float radius = 128.0f;
            if (obj.properties.count("radius")) {
                auto prop = obj.properties.at("radius");
                if (prop.getType() == tson::Type::Int) radius = (float)prop.getValue<int>();
                else if (prop.getType() == tson::Type::Float) radius = prop.getValue<float>();
            }

            // 3. Spawn tepat 1 musuh di tengah objek spawn
            Vector2 spawnPos = { obj.bounds.x + obj.bounds.width / 2.0f, obj.bounds.y + obj.bounds.height / 2.0f };
            
            Enemy *enemy = new Enemy();
            enemy->Init(spawnPos, obj.name.c_str(), type, radius);
            Entities::AddDynamic(enemy);
            
            TraceLog(LOG_INFO, "ENEMY: Created 1 enemy (Type: %d) from spawn point '%s'", (int)type, obj.name.c_str());
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

    return state;
}

/*==============================================================================
 * Update
 *==============================================================================*/

/**
 * @brief Update ukuran window dan scale multiplier tiap frame
 *
 * Wajib dipanggil tiap frame sebelum rendering agar scaling tetap benar
 * ketika window di-resize oleh user.
 *
 * @param state Pointer ke GameState aktif
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
 * Semua update logic entity masuk lewat sini.
 * Tambahkan system baru di sini kalau ada entity atau module baru.
 */
void UpdateLogicAll()
{
    Entities::Update();
    Combat::UpdateDamagePopups(GetFrameTime());
}

/*==============================================================================
 * Rendering
 *==============================================================================*/

/**
 * @brief Entry point render — semua yang terlihat di layar lewat sini
 *
 * Urutan layer render (urutan penting, jangan diubah sembarangan):
 * 1. Map — tile-based, tidak kena transform camera
 * 2. Entities & world overlay — dalam BeginMode2D (world space)
 * 3. Debug panels — screen space, di luar BeginMode2D
 * 4. UI overlay — HUD, pause menu, dll
 *
 * @param state Pointer ke GameState aktif
 */
void DrawRenderTexture(GameState *state)
{
    BeginTextureMode(state->Dungeon);
    ClearBackground(RAYWHITE);

    // layer 1: tile map
    RenderMap();

    BeginMode2D(camera);
    Entities::Render();
    Combat::DrawDamagePopups();
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
 *
 * Dipanggil di dalam BeginTextureMode, setelah semua world-space rendering selesai.
 * Tambahkan UI baru di sini.
 *
 * @param state Pointer ke GameState aktif
 */
void DrawUIOverlay(GameState *state)
{
    DrawPlayerHUD();

    if (pauseMenu.IsActive())
    {
        Vector2 mousePos = GetVirtualMousePosition(state);
        pauseMenu.Draw(mousePos);
    }
}

/**
 * @brief Scale dan render layar virtual ke window asli
 *
 * Layar virtual 1280x720 di-fit ke ukuran window sambil menjaga aspect ratio.
 * Area yang tidak terpakai diisi warna hitam (letterbox).
 *
 * @param state Pointer ke GameState aktif
 */
void DrawRenderWindows(GameState *state)
{
    // hitung offset centering biar layar virtual di tengah window
    float offsetX = (state->WindowScreenWidth - ((float)GameScreenWidth * state->ScaleMultiplier)) * 0.5F;
    float offsetY = (state->WindowScreenHeight - ((float)GameScreenHeight * state->ScaleMultiplier)) * 0.5F;

    BeginDrawing();
    ClearBackground(BLACK);

    DrawTexturePro(
        state->Dungeon.texture,
        (Rectangle){0, 0, (float)GameScreenWidth, -(float)GameScreenHeight}, // source flipped Y
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
 *
 * Diperlukan karena game dirender ke texture virtual yang di-scale ke window.
 * Tanpa konversi ini, posisi mouse tidak akan akurat di virtual screen.
 *
 * @param state Pointer ke GameState (butuh ScaleMultiplier dan ukuran window)
 * @return Vector2 posisi mouse dalam koordinat virtual screen (0,0 sampai 1280x720)
 */
Vector2 GetVirtualMousePosition(GameState *state)
{
    Vector2 mouse = GetMousePosition();
    Vector2 virtualMouse = {0, 0};

    // kurangi offset letterbox lalu bagi scale untuk dapat koordinat virtual
    virtualMouse.x = (mouse.x - ((state->WindowScreenWidth - (GameScreenWidth * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;
    virtualMouse.y = (mouse.y - ((state->WindowScreenHeight - (GameScreenHeight * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;

    return Vector2Clamp(virtualMouse, (Vector2){0, 0}, (Vector2){(float)GameScreenWidth, (float)GameScreenHeight});
}

/*==============================================================================
 * Cleanup
 *==============================================================================*/

/**
 * @brief Bersihin semua resource sebelum game ditutup
 *
 * Urutan cleanup (urutan penting):
 * 1. Unload semua texture di TexturesMap
 * 2. Unload map data
 * 3. Unload render texture virtual
 * 4. Tutup audio dan window
 *
 * @param state Pointer ke GameState aktif
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