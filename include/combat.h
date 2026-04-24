#pragma once
#include "raylib.h"
#include "damageQueue.h"
#include <vector>

class Player;

struct SwingAttack {
    bool active = false;
    float timer = 0.0f;
    float duration = 0.7f;      // Durasi ayunan lebih lambat (0.7 detik)
    float startAngle = 0.0f;
    float currentAngle = 0.0f;
    float sweepAngle = 180.0f;  // Ayunan 180 derajat
    Vector2 center = {0, 0};
    int iconX = 6;              // Default icon (Sword)
    int iconY = 4;
    std::vector<void*> damagedEntities; // List entitas yang sudah terkena damage dalam satu ayunan
    bool pressRegistered = false;       // Memastikan klik dimulai saat state PLAY
};

namespace Combat
{
    void HandleCombat(Player &player);
    void HandleRevive(Player &player);
    
    void UpdateSwingAttack(Player &player, float dt);
    void DrawSwingAttack(Player &player);

    void AddDamagePopup(Vector2 pos, float damage);
    void UpdateDamagePopups(float dt);
    void DrawDamagePopups();
}
