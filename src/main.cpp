#include "../include/dungeon.h"
#include "../include/screen.h"
#include "../include/map.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"

int main()
{
    GameState state = InitScreen();
    InitDrawMap(&state);

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