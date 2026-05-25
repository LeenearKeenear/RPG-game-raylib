#pragma once
#include "raylib.h"
#include "effects.h"
#include "item.h"
#include <vector>

class Player;

struct SwingAttack
{
    bool active = false;                    // Apakah serangan sedang aktif
    float timer = 0.0f;                     // Timer progres serangan
    float duration = 0.9f;                  // Durasi total serangan
    float startAngle = 0.0f;                // Sudut awal ayunan
    float currentAngle = 0.0f;              // Sudut ayunan saat ini
    float sweepAngle = 180.0f;              // Total sudut sapuan
    Vector2 center = {0, 0};                // Titik pusat serangan
    int iconX = 6;                          // Posisi X icon attack
    int iconY = 4;                          // Posisi Y icon attack
    std::vector<void *> damagedEntities;    // Entitas yang sudah terkena damage
    bool pressRegistered = false;           // Apakah press sudah tercatat
    AttackType type = ATTACK_SLASH;         // Tipe serangan
    float reach = 32.0f;                    // Jarak jangkauan serangan
    float breadth = 48.0f;                  // Lebar area serangan
    float thrustOffset = 0.0f;              // Offset thrust
    float baseAngle = 0.0f;                 // Sudut dasar serangan
    float raycastAngle = 0.0f;              // Sudut raycast serangan
    float damage = 25.0f;                   // Damage per serangan
    float knockbackForce = 1.0f;            // Kekuatan knockback
};

namespace Combat
{
    /** @brief Proses combat player setiap frame */
    void HandleCombat(Player &player);
    /** @brief Proses revive player */
    void HandleRevive(Player &player);
    /** @brief Update animasi swing attack */
    void UpdateSwingAttack(Player &player, float dt);
    /** @brief Render visual swing attack */
    void DrawSwingAttack(Player &player);
    /** @brief Tambahkan damage popup di posisi tertentu */
    void AddDamagePopup(Vector2 pos, float damage);
}
