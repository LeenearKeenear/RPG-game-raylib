#pragma once
#include "raylib.h"

class Player;

namespace Combat
{
    void HandleCombat(Player &player);
    void HandleRevive(Player &player);
}
