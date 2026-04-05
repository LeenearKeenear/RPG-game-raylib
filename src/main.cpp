#include "../include/screen.h"
#include "../raylib/include/raylib.h"

int main(void)
{
    GameState Game = InitScreen();

    while (!WindowShouldClose())
    {
        UpdateGameState(&Game);

        DrawRenderTexture(&Game);
        DrawRenderWindow(&Game);
    }

    UnloadRenderTexture(Game.Dungeon);
    CloseWindow();

    return 0;
}