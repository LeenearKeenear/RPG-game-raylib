#pragma once
#include <cmath>
#include "../lib/raylib/include/raylib.h"
#include "Map.h"
#include "Player.h"

enum EnemyState {
    IDLE,
    ROAMING,
    CHASE,
    ATTACK,
    DEAD
};

struct Enemy{
    Vector2 position;
    Direction Direction = DOWN;

    float speed;
    float detectionrange;
    float attackrange;

    float roamtimer;

    EnemyState state;
};