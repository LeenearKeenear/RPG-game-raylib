#include "include/screen.h"
#include <raylib.h>
#include <raymath.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

const float ScaleMultiplierMonitor = 0.7f;
const float ScaleMinMultiplierMonitor = 0.4f; 
const int GameScreenWidth = 1280;
const int GameScreenHeight = 720;

// inisialisasi screen
GameState InitScreen(void) {
    GameState state = {0};

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Dungeon Game"); // sementara pakai default dulu

    state.WindowScreenWidth  = (int)(GetMonitorWidth(0)  * ScaleMultiplierMonitor);
    state.WindowScreenHeight = (int)(GetMonitorHeight(0) * ScaleMultiplierMonitor);
    state.ScaleMultiplier    = MIN(
        (float)state.WindowScreenWidth  / GameScreenWidth,
        (float)state.WindowScreenHeight / GameScreenHeight
    );

    SetWindowSize(state.WindowScreenWidth, state.WindowScreenHeight);
    SetWindowMinSize(
        (int)(GetMonitorWidth(0)  * ScaleMinMultiplierMonitor),
        (int)(GetMonitorHeight(0) * ScaleMinMultiplierMonitor)
    );

    state.Dungeon = LoadRenderTexture(GameScreenWidth, GameScreenHeight);
    SetTextureFilter(state.Dungeon.texture, TEXTURE_FILTER_BILINEAR);
    SetTargetFPS(60);

    return state;
}

void UpdateGame(GameState *state) {
    state->WindowScreenWidth  = GetScreenWidth();
    state->WindowScreenHeight = GetScreenHeight();
    state->ScaleMultiplier    = MIN(
        (float)state->WindowScreenWidth  / GameScreenWidth,
        (float)state->WindowScreenHeight / GameScreenHeight
    );
}

void DrawRenderTexture(GameState *state) {
    Vector2 Mouse = GetMousePosition();
    Vector2 virtualMouse = {0};
    virtualMouse.x = (Mouse.x - (state->WindowScreenWidth  - (GameScreenWidth  * state->ScaleMultiplier)) * 0.5f) / state->ScaleMultiplier;
    virtualMouse.y = (Mouse.y - (state->WindowScreenHeight - (GameScreenHeight * state->ScaleMultiplier)) * 0.5f) / state->ScaleMultiplier;
    virtualMouse   = Vector2Clamp(virtualMouse, (Vector2){0, 0}, (Vector2){(float)GameScreenWidth, (float)GameScreenHeight});

    BeginTextureMode(state->Dungeon);
        ClearBackground(RAYWHITE);
        DrawRectangle(0, 0, GameScreenWidth, GameScreenHeight, RED);
        DrawText("If executed inside a window,\nyou can resize the window,\nand see the screen scaling!", 10, 25, 20, WHITE);
        DrawText(TextFormat("Default Mouse: [%i , %i]", (int)Mouse.x,        (int)Mouse.y),        350, 25, 20, GREEN);
        DrawText(TextFormat("Virtual Mouse: [%i , %i]", (int)virtualMouse.x, (int)virtualMouse.y), 350, 55, 20, YELLOW);
    EndTextureMode();
}

void DrawRenderWindows(GameState *state) {
    float offsetX = (state->WindowScreenWidth  - (float)GameScreenWidth  * state->ScaleMultiplier) * 0.5f;
    float offsetY = (state->WindowScreenHeight - (float)GameScreenHeight * state->ScaleMultiplier) * 0.5f;

    BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(
            state->Dungeon.texture,
            (Rectangle){ 0, 0, (float)GameScreenWidth, -(float)GameScreenHeight },
            (Rectangle){ offsetX, offsetY, (float)GameScreenWidth * state->ScaleMultiplier, (float)GameScreenHeight * state->ScaleMultiplier },
            (Vector2){ 0, 0 },
            0.0f,
            WHITE
        );
    EndDrawing();
}