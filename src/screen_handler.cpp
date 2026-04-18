/**
 * @file screen.cpp
 * @brief Implementasi dari Screen & GameState Management Module
 *
 * Implementasi dari fungsi-fungsi screen yang dideklarasikan di screen.h
 * Handle virtual screen rendering, scaling, window management, dan game loop.
 */

#include "../include/screen.h"
#include "../include/map.h"
#include "../include/animation.h"
#include "../include/player.h"
#include "../include/entities.h"
#include "../include/debug.h"
#include "../include/pauseMenu.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/hud.h"

/*==============================================================================
 * External Variables & Macros
 *==============================================================================*/

/** Reference ke pause menu global (dari main.cpp) */
extern PauseMenu pauseMenu;

/** Macro min untuk perbandingan dua nilai */
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/*==============================================================================
 * Constants
 *==============================================================================*/

/**
 * @brief Konstanta layar virtual — semua rendering di-scale ke ukuran ini
 */
const float ScaleMultiplierMonitor = 0.7F;    // ukuran default window = 70% monitor
const float ScaleMinMultiplierMonitor = 0.4F; // ukuran minimum window = 40% monitor

/** Lebar layar virtual (semua rendering pake ukuran ini) */
extern const int GameScreenWidth = 1280;

/** Tinggi layar virtual (semua rendering pake ukuran ini) */
extern const int GameScreenHeight = 720;

/*==============================================================================
 * Initialization Functions
 *==============================================================================*/

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
    // Step 1: Inisialisasi player — spawn point diambil otomatis dari object layer Tiled
    PlayerInstance.Init(SPAWN_OBJECT_NAME);

    // Step 2: Set camera ke tengah spawn player
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
 * @return GameState yang udah diinisialisasi
 */
GameState InitScreen()
{
    GameState state = {{0}};

    // Step 1: Setup window (resizable, title)
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Dungeon Game");
    InitAudioDevice();

    // Step 2: Hitung ukuran default window (70% monitor)
    state.WindowScreenWidth = (int)(GetMonitorWidth(0) * ScaleMultiplierMonitor);
    state.WindowScreenHeight = (int)(GetMonitorHeight(0) * ScaleMultiplierMonitor);
    state.ScaleMultiplier = MIN(
        (float)state.WindowScreenWidth / GameScreenWidth,
        (float)state.WindowScreenHeight / GameScreenHeight);

    // Step 3: Set ukuran window dan minimum size (40% monitor)
    SetWindowSize(state.WindowScreenWidth, state.WindowScreenHeight);
    SetWindowMinSize(
        (int)(GetMonitorWidth(0) * ScaleMinMultiplierMonitor),
        (int)(GetMonitorHeight(0) * ScaleMinMultiplierMonitor));

    // Step 4: Buat render texture virtual (target rendering)
    state.Dungeon = LoadRenderTexture(GameScreenWidth, GameScreenHeight);
    SetTextureFilter(state.Dungeon.texture, TEXTURE_FILTER_BILINEAR);

    // Step 5: Set FPS target ke 60
    const int FPS = 60;
    SetTargetFPS(FPS);

    // Step 6: Set state awal ke MAIN_MENU
    state.currentScreen = MAIN_MENU;
    state.showFPS = false;

    return state;
}

/*==============================================================================
 * Game Loop & Update Functions
 *==============================================================================*/

/**
 * @brief UpdateGame()
 * Update ukuran window dan scale multiplier tiap frame.
 * Dipanggil tiap frame biar scaling tetap bener pas window di-resize.
 * @param state Pointer ke GameState
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
 * @brief UpdateLogicAll()
 * Entry point update logic — semua logic game lewat sini tiap frame.
 * @note Tambah logic baru di sini kalau ada entity/system baru
 */
void UpdateLogicAll()
{
    PlayerInstance.Tick();
}

/*==============================================================================
 * Render Functions
 *==============================================================================*/

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
 * @param state Pointer ke GameState
 */
void DrawRenderTexture(GameState *state)
{
    // Mulai render ke texture virtual
    BeginTextureMode(state->Dungeon);
    ClearBackground(RAYWHITE);

    // ===== Layer 1: Map (tile-based) =====
    RenderMap();

    // ===== Layer 2: Entities & Debug Overlay (world-space) =====
    BeginMode2D(camera);
    RenderEntities();
    DebugInstance.DrawWorldOverlay();
    EndMode2D();

    // ===== Layer 3: UI & Debug Panels (screen-space) =====
    DebugInstance.Toggle();
    DebugInstance.Draw();

    // ===== Layer 4: UI Overlay (pause menu, dll) =====
    DrawUIOverlay(state);

    EndTextureMode();
}

/**
 * @brief DrawUIOverlay()
 * Render UI elements (HUD, pause menu, etc) ke virtual screen.
 * Dipanggil setelah rendering game, sebelum EndTextureMode().
 * @param state Pointer ke GameState
 */
void DrawUIOverlay(GameState *state)
{
    // 1. HUD Player (Stats, Name)
    DrawPlayerHUD();

    // 2. FPS Counter (if enabled)
    if (state->showFPS) {
        int fps = GetFPS();
        char fpsText[16];
        snprintf(fpsText, sizeof(fpsText), "FPS: %d", fps);
        DrawText(fpsText, 10, 10, 20, GREEN);
    }

    // 3. Menus
    if (pauseMenu.IsActive()) {
        Vector2 mousePos = GetVirtualMousePosition(state);
        pauseMenu.Draw(mousePos);
    }
}

/**
 * @brief DrawRenderWindows()
 * Render layar virtual (1280x720) ke window asli dengan scaling.
 * Layar virtual di-fit ke ukuran window sambil jaga aspect ratio.
 * Sisi yang gak kepakai diisi black bar.
 * @param state Pointer ke GameState
 */
void DrawRenderWindows(GameState *state)
{
    // Hitung offset biar layar virtual di-center di window
    float offsetX = (state->WindowScreenWidth - ((float)GameScreenWidth * state->ScaleMultiplier)) * 0.5F;
    float offsetY = (state->WindowScreenHeight - ((float)GameScreenHeight * state->ScaleMultiplier)) * 0.5F;

    BeginDrawing();
    ClearBackground(BLACK); // Black bar di area yang gak kepakai

    // Render texture virtual ke window dengan scaling
    DrawTexturePro(
        state->Dungeon.texture,
        (Rectangle){0, 0, (float)GameScreenWidth, -(float)GameScreenHeight}, // Source (flipped Y)
        (Rectangle){offsetX, offsetY, (float)GameScreenWidth * state->ScaleMultiplier, (float)GameScreenHeight * state->ScaleMultiplier},
        (Vector2){0, 0},
        0.0F,
        WHITE);
    EndDrawing();
}

/*==============================================================================
 * Utility Functions
 *==============================================================================*/

/**
 * @brief GetVirtualMousePosition()
 * Konversi koordinat mouse dari window ke virtual screen.
 * @param state Pointer ke GameState (butuh ScaleMultiplier dan window size)
 * @return Vector2 posisi mouse dalam koordinat virtual screen (1280x720)
 */
Vector2 GetVirtualMousePosition(GameState *state)
{
    Vector2 mouse = GetMousePosition();
    Vector2 virtualMouse = {0, 0};

    // Konversi: kurangi offset letterbox, lalu bagi dengan scale
    virtualMouse.x = (mouse.x - ((state->WindowScreenWidth - (GameScreenWidth * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;
    virtualMouse.y = (mouse.y - ((state->WindowScreenHeight - (GameScreenHeight * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;

    // Clamp ke batas virtual screen biar gak keluar
    return Vector2Clamp(virtualMouse, (Vector2){0, 0}, (Vector2){(float)GameScreenWidth, (float)GameScreenHeight});
}

/*==============================================================================
 * Cleanup Functions
 *==============================================================================*/

/**
 * @brief GameShutDown()
 * Bersihin semua resource sebelum game ditutup.
 *
 * @param state Pointer ke GameState
 */
void GameShutDown(GameState *state)
{
    // Unload semua texture yang ada di global TexturesMap
    for (int i = 0; i < MAX_TEXTURES; i++)
    {
        UnloadTexture(TexturesMap[i]);
    }

    // Unload map data dan render texture
    UnloadMap();
    UnloadRenderTexture(state->Dungeon);

    // Tutup audio dan window
    CloseAudioDevice();
    CloseWindow();
}

/*==============================================================================
 * Window & Video Settings Functions
 *==============================================================================*/

/**
 * @brief ToggleFullscreenMode()
 * Toggle antara fullscreen dan windowed mode.
 * @note Setelah toggle,UpdateGame() akan recalculate scale
 */
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

/**
 * @brief SetResolution()
 * Set ukuran window ke resolusi tertentu.
 * @param width Lebar window baru
 * @param height Tinggi window baru
 */
void SetResolution(int width, int height)
{
    SetWindowSize(width, height);
}

/**
 * @brief GetCurrentResolution()
 * Ambil resolusi saat ini.
 * @return Rectangle berisi x=width, y=height
 */
Rectangle GetCurrentResolution(void)
{
    Rectangle res = {0};
    res.width = static_cast<float>(GetScreenWidth());
    res.height = static_cast<float>(GetScreenHeight());
    return res;
}

/**
 * @brief GetMonitorResolution()
 * Ambil resolusi monitor utama.
 * @return Rectangle berisi x=monitorWidth, y=monitorHeight
 */
Rectangle GetMonitorResolution(void)
{
    Rectangle res = {0};
    res.width = static_cast<float>(GetMonitorWidth(0));
    res.height = static_cast<float>(GetMonitorHeight(0));
    return res;
}

/**
 * @brief IsFullscreen()
 * Cek apakah sedang dalam mode fullscreen.
 * @return true kalo fullscreen
 */
bool IsFullscreen(void)
{
    return IsWindowFullscreen();
}