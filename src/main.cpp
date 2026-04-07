<<<<<<< HEAD
// #include "../include/screen.h"
// #include "../include/map.h"
// #include "../lib/raylib/include/raylib.h"
// #include "../lib/raylib/include/raymath.h"
=======
#include "../include/screen.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
>>>>>>> main

// int main(void)
// {
//     GameState state = InitScreen();
//     InitDrawMap(&state);

//     while (!WindowShouldClose())
//     {
//         UpdateGame(&state);
//         UpdatePlayer(&state);
//         DrawRenderTexture(&state);
//         DrawRenderWindows(&state);
//     }

//     GameShutDown(&state);
//     return 0;
// }

#include "../include/game.h"

int main()
{
<<<<<<< HEAD
    Game Game;
    Game.Init();
=======
    GameState state = InitScreen();
    InitDrawMap(&state);
    InitAll();
>>>>>>> main

    while (!WindowShouldClose())
    {
        Game.Update();
        Game.Draw();
    }

    Game.Close();

    return 0;
}