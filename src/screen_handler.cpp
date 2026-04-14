#include "../include/screen.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../include/entities.h"
#include "../include/debug.h"
#include "../include/frustum.h"
#include "../include/pauseMenu.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"

extern PauseMenu pauseMenu;

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/**
 * @brief Konstanta layar virtual — semua rendering di-scale ke ukuran ini 
 */
const float ScaleMultiplierMonitor = 0.7F;    // ukuran default window = 70% monitor
const float ScaleMinMultiplierMonitor = 0.4F; // ukuran minimum window = 40% monitor
extern const int GameScreenWidth = 1280;
extern const int GameScreenHeight = 720;

/**
 * @brief InitAll()
 * Inisialisasi semua entity dan camera di awal game.
 * 
 * Cara kerja:
 * 1. Init player — spawn point otomatis dibaca dari object layer Tiled
 * 2. Set camera target ke posisi spawn player
 */
void InitAll()
{
    // inisialisasi player — spawn point diambil otomatis dari object layer Tiled
    PlayerInstance.Init();

    // set camera ke tengah spawn player
    Vector2 spawnPos = PlayerInstance.GetPosition();
    camera.target = {spawnPos.x + (TILE_SIZE / 2.0F), spawnPos.y + (TILE_SIZE / 2.0F)};
    camera.offset = {(float)(GameScreenWidth / 2), (float)(GameScreenHeight / 2)};
    camera.rotation = 0;
    camera.zoom = 1.0F;
}

/**
 * @brief InitScreen() 
 * Inisialisasi window, audio, dan render texture virtual.
 * 
 * Cara kerja:
 * 1. Buat window resizable ukuran 70% monitor
 * 2. Buat render texture 1280x720 sebagai layar virtual
 * 3. Set FPS target ke 60
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

    // ukuran default window = 70% monitor, minimum = 40% monitor
    SetWindowSize(state.WindowScreenWidth, state.WindowScreenHeight);
    SetWindowMinSize(
        (int)(GetMonitorWidth(0) * ScaleMinMultiplierMonitor),
        (int)(GetMonitorHeight(0) * ScaleMinMultiplierMonitor));

    state.Dungeon = LoadRenderTexture(GameScreenWidth, GameScreenHeight);
    SetTextureFilter(state.Dungeon.texture, TEXTURE_FILTER_BILINEAR);
    
    // Pastikan FPS tetap 60
    const int FPS = 60; 
    SetTargetFPS(FPS);

    state.currentScreen = MAIN_MENU;

    return state;
}

/** 
 * @brief UpdateGame()
 * Update ukuran window dan scale multiplier tiap frame.
 * Dipanggil tiap frame biar scaling tetap bener pas window di-resize.
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
 * @brief DrawRenderTexture() 
 * Entry point render — semua yang keliatan di layar lewat sini.
 * 
 * Urutan render:
 * 1. RenderMap() — render tile map dari Tiled
 * 2. RenderEntities() — render semua entity dalam world space (pake camera)
 * 3. DebugInstance — toggle dan draw debug panel kalau aktif
 * 
 * Catatan: debug panel di luar BeginMode2D biar posisinya fixed di layar
 */
void DrawRenderTexture(GameState *state)
{
    BeginTextureMode(state->Dungeon);
    ClearBackground(RAYWHITE);

    // RenderMap();
    RenderMapCulled();

    BeginMode2D(camera);
    RenderEntities();
    EndMode2D();

    DebugInstance.Toggle();
    DebugInstance.Draw();

    DrawUIOverlay(state);

    EndTextureMode();
}

/**
 * @brief DrawUIOverlay()
 * Render UI elements (pause menu, etc) ke virtual screen.
 * Dipanggil setelah rendering game, sebelum EndTextureMode().
 */
void DrawUIOverlay(GameState* state)
{
    if (pauseMenu.IsActive()) {
        Vector2 mousePos = GetVirtualMousePosition(state);
        pauseMenu.Draw(mousePos);
    }
}

/**
 * @brief GetVirtualMousePosition()
 * Konversi koordinat mouse dari window ke virtual screen.
 */
Vector2 GetVirtualMousePosition(GameState* state)
{
    Vector2 mouse = GetMousePosition();
    Vector2 virtualMouse = {0, 0};
    virtualMouse.x = (mouse.x - ((state->WindowScreenWidth - (GameScreenWidth * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;
    virtualMouse.y = (mouse.y - ((state->WindowScreenHeight - (GameScreenHeight * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;
    return Vector2Clamp(virtualMouse, (Vector2){0, 0}, (Vector2){(float)GameScreenWidth, (float)GameScreenHeight});
}

/**
 * @brief UpdateLogicAll()
 * Entry point update logic — semua logic game lewat sini tiap frame.
 * Tambah logic baru di sini kalau ada entity/system baru.
 */
void UpdateLogicAll()
{
    PlayerInstance.Tick();
}

/**
 * @brief DrawRenderWindows()
 * Render layar virtual (1280x720) ke window asli dengan scaling.
 * Layar virtual di-fit ke ukuran window sambil jaga aspect ratio.
 * Sisi yang gak kepakai diisi black bar.
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

/**
 * @brief GameShutDown()
 * Bersihin semua resource sebelum game ditutup.
 * 
 * TODO: Kalau nanti tiap map punya texture sendiri,
 * unload harus per map, bukan cuma loop MAX_TEXTURES global
 */
void GameShutDown(GameState *state)
{
    for (int i = 0; i < MAX_TEXTURES; i++) {
        UnloadTexture(TexturesMap[i]);
    }

    UnloadMap();
    UnloadRenderTexture(state->Dungeon);
    CloseAudioDevice();
    CloseWindow();
}