#pragma once
#include "raylib.h"
#include "effects.h"
#include "item.h"
#include <vector>

constexpr float ANGLE_UP_MIN = -135.0f;
constexpr float ANGLE_UP_MAX = -45.0f;
constexpr float ANGLE_RIGHT_MIN = -45.0f;
constexpr float ANGLE_RIGHT_MAX = 45.0f;
constexpr float ANGLE_DOWN_MIN = 45.0f;
constexpr float ANGLE_DOWN_MAX = 135.0f;
constexpr float THRUST_MAX_OFFSET = 16.0f;
constexpr float PIERCE_MAX_OFFSET = 24.0f;
constexpr float SLAM_MAX_OFFSET = 8.0f;
constexpr float WEAPON_ANGLE_RIGHT = -9.0f;
constexpr float WEAPON_ANGLE_LEFT = 189.0f;
constexpr float WEAPON_ANGLE_UP_RIGHT = -60.0f;
constexpr float WEAPON_ANGLE_UP_LEFT = -120.0f;
constexpr float WEAPON_ANGLE_DOWN_RIGHT = 60.0f;
constexpr float WEAPON_ANGLE_DOWN_LEFT = 120.0f;
constexpr float WEAPON_OFFSET_RIGHT_FACTOR = 0.8f;
constexpr float WEAPON_RENDER_ORIGIN_X = 17.0f;
constexpr float WEAPON_ROTATION_OFFSET = 90.0f;
constexpr float SLASH_PHASE_1_START = 1.0f / 3.0f;
constexpr float SLASH_PHASE_2_START = 2.0f / 3.0f;
constexpr float SLASH_DISTANCE_MULTIPLIER = 0.65f;
constexpr float SWORD1_SLASH_OFFSET_H = 26.0f;
constexpr float SWORD2_SLASH_OFFSET_H = 37.0f;
constexpr float SWORD1_SLASH_BACK_DIST = 16.0f;
constexpr float SWORD2_SLASH_BACK_DIST = 19.5f;

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
