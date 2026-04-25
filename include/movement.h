#pragma once
#include "raylib.h"

class Player;

namespace Movement
{
    void HandleMovement(Player &player);
    void UpdateCamera(Player &player);
    bool CanMove(Player &player, Vector2 newPos);
}
