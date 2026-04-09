#include "../include/game.h"

int main()
{
    Game Game;

    Game.Open();
    Game.Loop();
    Game.Close();

    return 0;
}