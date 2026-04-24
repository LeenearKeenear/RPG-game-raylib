#pragma once
#include "raylib.h"

class Player;

struct SwingAttack {
    bool active = false;
    float timer = 0.0f;
    float duration = 0.2f;      // Durasi ayunan singkat (snappy)
    float startAngle = 0.0f;
    float currentAngle = 0.0f;
    float sweepAngle = 180.0f;  // Ayunan 180 derajat
    Vector2 center = {0, 0};
    int iconX = 6;              // Default icon (Sword)
    int iconY = 4;
};

namespace Combat
{
    void HandleCombat(Player &player);
    void HandleRevive(Player &player);
    
    void UpdateSwingAttack(Player &player, float dt);
    void DrawSwingAttack(Player &player);
}

