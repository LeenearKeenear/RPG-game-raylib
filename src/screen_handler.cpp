#include "../include/screen.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../include/entities.h"
#include "../include/debug.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// variabel konstanta
const float ScaleMultiplierMonitor = 0.7F;
const float ScaleMinMultiplierMonitor = 0.4F;
extern const int GameScreenWidth = 1280;
extern const int GameScreenHeight = 720;

// inisialisasi semua entity dan camera
void InitAll(void)
{
    // inisialisasi player — spawn point diambil otomatis dari object layer Tiled
    PlayerInstance.Init();

    // inisialisasi camera
    Vector2 spawnPos = PlayerInstance.GetPosition();
    camera.target = {spawnPos.x + (TILE_SIZE / 2.0f), spawnPos.y + (TILE_SIZE / 2.0f)};
    camera.offset = {(float)(GameScreenWidth / 2), (float)(GameScreenHeight / 2)};
    camera.rotation = 0;
    camera.zoom = 1.0f;
}

// inisialisasi screen
GameState InitScreen(void)
{
    GameState state = {{0}};

    SetConfigFlags(FLAG_WINDOW_RESIZABLE); // ini yang ngatur windows bisa di resize
    InitWindow(1280, 720, "Dungeon Game"); // inisialisasi windows pertama
    InitAudioDevice();                     // inisialisasi audio

    state.WindowScreenWidth = (int)(GetMonitorWidth(0) * ScaleMultiplierMonitor);
    state.WindowScreenHeight = (int)(GetMonitorHeight(0) * ScaleMultiplierMonitor);
    state.ScaleMultiplier = MIN(
        (float)state.WindowScreenWidth / GameScreenWidth,
        (float)state.WindowScreenHeight / GameScreenHeight);

    SetWindowSize(state.WindowScreenWidth, state.WindowScreenHeight); // default windows size
    SetWindowMinSize(
        (int)(GetMonitorWidth(0) * ScaleMinMultiplierMonitor),
        (int)(GetMonitorHeight(0) * ScaleMinMultiplierMonitor)); // maksimum ukuran windows yang bisa di kecilin

    state.Dungeon = LoadRenderTexture(GameScreenWidth, GameScreenHeight);
    SetTextureFilter(state.Dungeon.texture, TEXTURE_FILTER_BILINEAR); // ini yang ngatur jenis renderingnya
    SetTargetFPS(60);

    state.currentScreen = MAIN_MENU;

    return state;
}

// buat update isi virtualisasinya
void UpdateGame(GameState *state)
{
    state->WindowScreenWidth = GetScreenWidth();
    state->WindowScreenHeight = GetScreenHeight();
    state->ScaleMultiplier = MIN(
        (float)state->WindowScreenWidth / GameScreenWidth,
        (float)state->WindowScreenHeight / GameScreenHeight);
}

// isi dari virtualisasinya dan ini bakal jadi entry point untuk semua jenis menu
void DrawRenderTexture(GameState *state)
{
    BeginTextureMode(state->Dungeon);
    ClearBackground(RAYWHITE);

    TilesonRender(state);

    BeginMode2D(camera);
    RenderEntities();
    EndMode2D();

    DebugInstance.Toggle();
    DebugInstance.Draw();

    EndTextureMode();
}

// isinya buat update logic buat semuanya
void UpdateLogicAll(GameState *state)
{
    PlayerInstance.Tick();
}

// buat render layar windows utamanya
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

// buat matiin game dan ngebebasin semua memory yang ada
void GameShutDown(GameState *state)
{
    // TODO MULTI-MAP: kalau nanti tiap map punya texture sendiri
    // unload harus per map, bukan cuma loop MAX_TEXTURES global
    for (int i = 0; i < MAX_TEXTURES; i++)
        UnloadTexture(TexturesMap[i]);

    TilesonUnloadMap();
    UnloadRenderTexture(state->Dungeon);
    CloseAudioDevice();
    CloseWindow();
}
