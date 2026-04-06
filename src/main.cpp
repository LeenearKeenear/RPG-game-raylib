#include "../include/player.h"
#include "../include/screen.h"
#include "../include/map.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"

int main(void)
{
    GameState state = InitScreen();
    InitDrawMap(&state);
    InitAll();

    while (!WindowShouldClose())
    {
        UpdateGame(&state);
        UpdatePlayer(&state);
        DrawRenderTexture(&state);
        DrawRenderWindows(&state);
    }

    GameShutDown(&state);
    return 0;
}