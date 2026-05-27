#pragma once
#include "raylib.h"
#include "effects.h"
#include "item.h"
#include <vector>

class Player;
class Entity;

namespace Combat
{
    struct Attack
    {
        bool active = false;
        float timer = 0.0f;
        float duration = 0.0f;
        float raycastAngle = 0.0f;
        Vector2 center = {0, 0};
        std::vector<Entity *> damagedEntities;
        bool pressHeld = false;
        const WeaponData* weapon = nullptr;
    };

    void Update(Player &player);
    void HandleDead(Player &player);
    void HandleStamina(Player &player);
    void HandleAttack(Player &player);

    // void HandleCombat(Player &player);
    // void HandleRevive(Player &player);
    // void UpdateSwingAttack(Player &player, float dt);
    // void DrawSwingAttack(Player &player);
    // void AddDamagePopup(Vector2 pos, float damage);
}
