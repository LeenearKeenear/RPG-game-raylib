#include "../include/dungeon.h"
#include "../include/screen.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"

int main(void) {
    GameState state = InitScreen();

    while (!WindowShouldClose()) {
        UpdateGame(&state);
        DrawRenderTexture(&state);
        DrawRenderWindows(&state);
    }

    UnloadRenderTexture(state.Dungeon);
    CloseWindow();
    return 0;
}