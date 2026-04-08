#include "../include/game.h"

int main()
{
    Game Game;

    Game.Init();
    Game.Run();
    Game.Shutdown();

    return 0;
}