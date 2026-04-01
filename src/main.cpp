#include "dungeon.h"
#include "screen.h"
#include <raylib.h>
#include <raymath.h>

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