#pragma once
#include "raylib.h"

class Player;

namespace Interaction
{
    void HandleInteractions(Player &player);
    void ExecutePendingTransitions(Player &player);
    void UpdateRaycast(Player &player);
    void CheckDoors(Player &player);
    void CheckProps(Player &player);
}
