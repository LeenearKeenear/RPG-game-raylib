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
    ENEMY_ATTACK,
    ENEMY_RETURN
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
     * @param radius Jarak patroli maksimal dari titik spawn
     */
    void Init(Vector2 pos, const char* name, EnemyType type = SLIME, float radius = 128.0f);

    void Update() override;
    void Render() override;
    void TakeDamage(float amount, Vector2 knockback = {0, 0}) override;

    // Logic AI
    void UpdateAI();
    bool CheckPlayerLoS();

    EnemyAIState AIState = ENEMY_IDLE;
    float BaseDetectionRange = 120.0f; // Jarak deteksi standar
    float ChaseDetectionRange = 240.0f; // Jarak deteksi saat mengejar (lebih besar)
    float DetectionRange = 120.0f;      // Jarak deteksi aktif
    float AttackRange = 16.0f;    // Jarak serangan (pixel)
    float Speed = 1.0f;           // Kecepatan gerak musuh
    float HealthRegenRate = 10.0f; // Kecepatan pengisian darah (sama dengan rate player)

    Animation Anim;
    std::string Name;

    Vector2 PatrolTarget;         // Titik tujuan patroli
    Vector2 SpawnPoint;           // Titik pusat spawn/area patroli
    float PatrolRadius = 128.0f;  // Radius maksimal dari SpawnPoint
    float PatrolTimer = 0.0f;     // Timer untuk jeda patroli
    const float PatrolWaitTime = 2.0f;

    // Hitbox data
    float HitboxWidth = 16.0f;
    float HitboxHeight = 12.0f;
    float HitboxOffsetX = 8.0f;
    float HitboxOffsetY = 14.0f;
    Rectangle GetHitbox() const override { return { Position.x + HitboxOffsetX, Position.y + HitboxOffsetY, HitboxWidth, HitboxHeight }; }

    EnemyType Type = SLIME;
    const AnimationSet* AnimSet = &SlimeAnimationSet;

private:
    void HandleIdle();
    void HandlePatrol();
    void HandleChase();
    void HandleAttack();
    void HandleReturn();
    void PerformAttack();

    RayCast Ray;

    float AttackCooldownTimer = 0.0f;
    const float AttackCooldown = 1.0f;
    bool PlayerWasInRange = false;

    // Feedback visual/physics
    float HitFlashTimer = 0.0f;
    Vector2 KnockbackVelocity = {0, 0};
    float DeathTimer = 0.0f;
    const float DeathDuration = 1.2f;
};
