#pragma once

#include "entity.h"
#include "animation.h"
#include "mapLogic.h"
#include <string>

/**
 * @brief Status AI untuk perilaku Musuh (FSM).
 */
enum EnemyAIState {
    ENEMY_IDLE,    ///< Berdiri diam atau menunggu
    ENEMY_PATROL,  ///< Bergerak antar titik acak di sekitar titik spawn
    ENEMY_CHASE,   ///< Mengejar pemain
    ENEMY_ATTACK,  ///< Menjalankan animasi/logika serangan
    ENEMY_RETURN   ///< Kembali ke titik spawn setelah kehilangan pemain
};

/**
 * @brief Berbagai jenis musuh dengan visual/statistik unik.
 */
enum EnemyType {
    SLIME,
    SKELETON,
    WOLF
};

/**
 * @brief Kelas Musuh yang mengimplementasikan logika AI dan manajemen status.
 * Mewarisi dari Entity untuk properti dasar.
 */
class Enemy : public Entity {
public:
    Enemy();
    virtual ~Enemy();

    /**
     * @brief Inisialisasi musuh dalam koordinat dunia (world space).
     */
    void Init(Vector2 pos, const char* name, int mapId, EnemyType type = SLIME, float radius = 128.0f);

    void Update() override;
    void Render() override;
    void TakeDamage(float amount, Vector2 knockback = {0, 0}) override;

    void UpdateAI();         ///< Titik masuk utama logika AI
    bool CheckPlayerLoS();   ///< Memeriksa Line of Sight (jarak pandang) ke pemain menggunakan raycasting

    EnemyAIState AIState = ENEMY_IDLE;
    float BaseDetectionRange = 120.0f; ///< Jarak deteksi saat idle/patroli
    float ChaseDetectionRange = 240.0f; ///< Jarak deteksi saat sedang mengejar (lebih sulit kabur)
    float DetectionRange = 120.0f;      ///< Jarak pandang aktif
    float AttackRange = 16.0f;          ///< Jarak yang dibutuhkan untuk memicu serangan
    float Damage = 5.0f;                ///< Damage yang dihasilkan per pukulan
    float Speed = 1.0f;                 ///< Kecepatan gerak saat patroli/kembali
    float ChaseSpeed = 1.5f;            ///< Kecepatan gerak saat mengejar
    float HealthRegenRate = 10.0f;      ///< HP yang pulih per detik saat tidak dalam pertempuran
    float HealthRegenTimer = 0.0f;     ///< Timer untuk jeda pemulihan HP
    const float HealthRegenDelay = 2.0f;

    Animation Anim;                     ///< Pengontrol animasi
    std::string Name;                   ///< Nama musuh
    int MapObjectID = -1;               ///< ID Objek Tiled untuk persistensi kematian

    Vector2 PatrolTarget;               ///< Koordinat tujuan patroli saat ini
    Vector2 SpawnPoint;                 ///< Lokasi spawn awal (pusat area patroli)
    float PatrolRadius = 128.0f;        ///< Seberapa jauh musuh bisa berkeliaran
    float PatrolTimer = 0.0f;           ///< Timer untuk menunggu di titik patroli
    const float PatrolWaitTime = 2.0f;

    // Konfigurasi Hitbox
    float HitboxWidth = 16.0f;
    float HitboxHeight = 12.0f;
    float HitboxOffsetX = 8.0f;
    float HitboxOffsetY = 14.0f;
    Rectangle GetHitbox() const override { return { Position.x + HitboxOffsetX, Position.y + HitboxOffsetY, HitboxWidth, HitboxHeight }; }

    EnemyType Type = SLIME;
    const AnimationSet* AnimSet = &SlimeAnimationSet;

private:
    // Metode penanganan status (state handlers)
    void HandleIdle();
    void HandlePatrol();
    void HandleChase();
    void HandleAttack();
    void HandleReturn();
    void PerformAttack();

    RayCast Ray;                        ///< Raycast untuk pemeriksaan LoS dan pergerakan

    float AttackCooldownTimer = 0.0f;
    const float AttackCooldown = 1.0f;
    bool PlayerWasInRange = false;

    // Feedback dan status kematian
    float HitFlashTimer = 0.0f;
    Vector2 KnockbackVelocity = {0, 0};
    float DeathTimer = 0.0f;
    const float DeathDuration = 1.2f;
};

// Utility functions
int GetRandomDamage(int min, int max);
void InitEnemy();
void InitEnemyTextures();
void SpawnRandomWave();
void SpawnRandomEnemy();
void SaveEnemiesForMap(const std::string& mapPath);
bool LoadEnemiesForMap(const std::string& mapPath);
void ClearEnemies();
