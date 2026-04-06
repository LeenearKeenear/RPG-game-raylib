#include "../include/dungeon.h"
#include "../include/screen.h"
#include "../include/map.h"
#include "../include/mainMenu.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"

int main()
{
    GameState state = InitScreen();
    InitDrawMap(&state);
    InitMainMenu(&state);

    while (!WindowShouldClose())
    {
        if (state.currentScreen == MAIN_MENU)
        {
            UpdateMainMenu(&state);
            DrawMainMenu(&state);
        }
        else if (state.currentScreen == PLAY)
        {
            UpdateGame(&state);
            UpdatePlayer(&state);
            DrawRenderTexture(&state);
            DrawRenderWindows(&state);
        }
    }

    GameShutDown(&state);
    return 0;
}
