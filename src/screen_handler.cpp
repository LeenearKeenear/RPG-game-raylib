#include "../include/screen.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// variabel konstanta
const float ScaleMultiplierMonitor = 0.7F;
const float ScaleMinMultiplierMonitor = 0.4F;
extern const int GameScreenWidth = 1280;
extern const int GameScreenHeight = 720;

void InitAll(void)
{
    // sementara
    // inisialisasi player potition
    Player = (Entity){
        .PlayerPosition = {CurrentMap->SpawnPointPlayer.x * TILE_WIDTH,
                           CurrentMap->SpawnPointPlayer.y * TILE_HEIGHT},
        .MoveTimer = 0.0f,
        .MoveDelay = 0.15,
    };

    // TODO MULTI-MAP: Door harusnya diambil dari data object layer map aktif
    // sementara
    Door = (sTile){
        .CoordinateTile = {TILE_WIDTH * 10, TILE_HEIGHT * 10},
    };

    // inisialisasi camera
    camera.target = (Vector2){(float)(CurrentMap->SpawnPointPlayer.x * TILE_WIDTH), (float)(CurrentMap->SpawnPointPlayer.y * TILE_HEIGHT)}; // ini targetin player biar ditengah map
    camera.offset = (Vector2){(float)(GameScreenWidth / 2), (float)(GameScreenHeight / 2)};                    // kamera di tengah map
    camera.rotation = {0};
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

    RenderMap(state);

    EndTextureMode();
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
    {
        UnloadTexture(TexturesMap[i]);
    }

    UnloadMap();
    UnloadRenderTexture(state->Dungeon);
    CloseAudioDevice();
    CloseWindow();
}

// isinya buat debug menu posisi mouse, sapa tau butuh kan
void DebugMouse(GameState *state)
{
    Vector2 Mouse = GetMousePosition();
    Vector2 virtualMouse = {0, 0};
    virtualMouse.x = (Mouse.x - ((state->WindowScreenWidth - (GameScreenWidth * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;
    virtualMouse.y = (Mouse.y - ((state->WindowScreenHeight - (GameScreenHeight * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;
    virtualMouse = Vector2Clamp(virtualMouse, (Vector2){0, 0}, (Vector2){(float)GameScreenWidth, (float)GameScreenHeight});

    DrawText(TextFormat("Default Mouse: [%i , %i]", (int)Mouse.x, (int)Mouse.y), 5, 240, 25, GREEN);
    DrawText(TextFormat("Virtual Mouse: [%i , %i]", (int)virtualMouse.x, (int)virtualMouse.y), 5, 265, 25, YELLOW);
}