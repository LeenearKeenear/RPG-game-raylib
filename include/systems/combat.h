#pragma once
#include "raylib.h"
#include "effects.h"
#include "item.h"
#include <vector>

class Player;

struct SwingAttack
{
    bool active = false;
    float timer = 0.0f;
    float duration = 0.9f;
    float startAngle = 0.0f;
    float currentAngle = 0.0f;
    float sweepAngle = 180.0f;
    Vector2 center = {0, 0};
    int iconX = 6;
    int iconY = 4;
    std::vector<void *> damagedEntities;
    bool pressRegistered = false;
    AttackType type = ATTACK_SLASH;
    float reach = 32.0f;
    float breadth = 48.0f;
    float thrustOffset = 0.0f;
    float baseAngle = 0.0f;
    float raycastAngle = 0.0f;
    float damage = 25.0f;
    float knockbackForce = 1.0f;
};

namespace Combat
{
    void HandleCombat(Player &player);
    void HandleRevive(Player &player);
    void UpdateSwingAttack(Player &player, float dt);
    void DrawSwingAttack(Player &player);
    void AddDamagePopup(Vector2 pos, float damage);
}
