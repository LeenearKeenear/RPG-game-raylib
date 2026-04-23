#pragma once

#include "entity.h"
#include "animation.h"
#include "mapLogic.h"
#include <string>

/**
 * @brief State AI untuk Musuh
 */
enum EnemyAIState {
    ENEMY_IDLE,
    ENEMY_PATROL,
    ENEMY_CHASE,
    ENEMY_ATTACK
};

enum EnemyType {
    SLIME,
    SKELETON,
    WOLF
};

/**
 * @brief Kelas Musuh yang mewarisi Entity
 * Memiliki sistem FSM (Finite State Machine) sederhana untuk perilaku AI.
 */
class Enemy : public Entity {
public:
    Enemy();
    virtual ~Enemy();

    /**
     * @brief Inisialisasi musuh di posisi tertentu
     * @param pos Posisi awal world space
     * @param name Nama musuh
     * @param type Tipe musuh (Slime, Skeleton, Wolf)
     */
    void Init(Vector2 pos, const char* name, EnemyType type = SLIME);

    void Update() override;
    void Render() override;

    // Logic AI
    void UpdateAI();
    bool CheckPlayerLoS();

    EnemyAIState AIState = ENEMY_IDLE;
    float DetectionRange = 160.0f; // Jarak deteksi pemain (pixel)
    float AttackRange = 16.0f;    // Jarak serangan (pixel)
    float Speed = 1.0f;           // Kecepatan gerak musuh

    Animation Anim;
    std::string Name;

    Vector2 PatrolTarget;         // Titik tujuan patroli
    float PatrolTimer = 0.0f;     // Timer untuk jeda patroli
    const float PatrolWaitTime = 2.0f;

    // Hitbox data
    float HitboxWidth = 16.0f;
    float HitboxHeight = 12.0f;
    float HitboxOffsetX = 8.0f;
    float HitboxOffsetY = 14.0f;

    EnemyType Type = SLIME;
    const AnimationSet* AnimSet = &SlimeAnimationSet;

private:
    void HandleIdle();
    void HandlePatrol();
    void HandleChase();
    void HandleAttack();
    void PerformAttack();

    RayCast Ray;

    float AttackCooldownTimer = 0.0f;
    const float AttackCooldown = 1.5f;
    bool PlayerWasInRange = false;
};
