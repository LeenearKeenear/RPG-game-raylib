// #include "../include/screen.h"
// #include "../include/map.h"
#include "../include/game.h"
#include "../lib/raylib/include/raylib.h"

int main()
{
    // GameState Game = InitScreen();
    // InitDrawMap(&Game);
    Game Game;
    Game.Init();

    while (!WindowShouldClose())
    {
        // UpdateGame(&Game);
        // UpdatePlayer(&Game);
        Game.Update();  
        Game.Draw();
        
        // DrawRenderTexture(&Game);
        // DrawRenderWindows(&Game);   
    }

    // GameShutDown(&Game);
    Game.Close();

    return 0;
}