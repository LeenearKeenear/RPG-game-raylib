#include "../include/screen.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// variabel konstanta
const float ScaleMultiplierMonitor = 0.7F;
const float ScaleMinMultiplierMonitor = 0.4F;
const int GameScreenWidth = 1280;
const int GameScreenHeight = 720;

// inisialisasi screen
GameState InitScreen(void)
{
    GameState state = {{0}};

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT); // ini yang ngatur windows bisa di resize
    InitWindow(1280, 720, "Dungeon Game");                   // inisialisasi windows pertama

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

// isi dari virtualisasinya (contoh aja)
void DrawRenderTexture(GameState *state)
{
    Vector2 Mouse = GetMousePosition();
    Vector2 virtualMouse = {0, 0};
    virtualMouse.x = (Mouse.x - ((state->WindowScreenWidth - (GameScreenWidth * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;
    virtualMouse.y = (Mouse.y - ((state->WindowScreenHeight - (GameScreenHeight * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;
    virtualMouse = Vector2Clamp(virtualMouse, (Vector2){0, 0}, (Vector2){(float)GameScreenWidth, (float)GameScreenHeight});

    BeginTextureMode(state->Dungeon);
    ClearBackground(RAYWHITE);
    DrawRectangle(0, 0, GameScreenWidth, GameScreenHeight, RED);
    DrawText("If executed inside a window,\nyou can resize the window,\nand see the screen scaling!", 10, 25, 20, WHITE);
    DrawText(TextFormat("Default Mouse: [%i , %i]", (int)Mouse.x, (int)Mouse.y), 350, 25, 20, GREEN);
    DrawText(TextFormat("Virtual Mouse: [%i , %i]", (int)virtualMouse.x, (int)virtualMouse.y), 350, 55, 20, YELLOW);
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