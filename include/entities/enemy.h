#pragma once

#include "entity.h"
#include "../lib/raylib/include/raylib.h"
#include "animation.h"
#include "mapLogic.h"
#include "enemy_ai.h"
#include <string>
#include <vector>
#include <unordered_map>

// Spawn constants
constexpr int SPAWN_PINPOINT_NORMAL_MIN = 1;
constexpr int SPAWN_PINPOINT_NORMAL_MAX = 10;
constexpr int SPAWN_PINPOINT_ELITE_MIN = 1;
constexpr int SPAWN_PINPOINT_ELITE_MAX = 5;
constexpr int SPAWN_RECT_NORMAL_MIN = 4;
constexpr int SPAWN_RECT_NORMAL_MAX = 20;
constexpr int SPAWN_RECT_ELITE_MIN = 2;
constexpr int SPAWN_RECT_ELITE_MAX = 10;
constexpr int SPAWN_RETRY_LIMIT = 100;

/*==============================================================================
 * Enums
 *==============================================================================*/

/// @brief Status AI untuk perilaku musuh (FSM).
enum EnemyAIState
{
    ENEMY_IDLE,   ///< Berdiri diam atau menunggu
    ENEMY_PATROL, ///< Bergerak antar titik acak di sekitar titik spawn
    ENEMY_CHASE,  ///< Mengejar pemain
    ENEMY_ATTACK, ///< Menjalankan animasi/logika serangan
    ENEMY_RETURN  ///< Kembali ke titik spawn setelah kehilangan pemain
};

enum RayCastMode
{
    LINE,
    CONE
};

typedef enum EnemyRank
{
    ENEMY_NORMAL,
    ENEMY_ELITE,
    ENEMY_BOSS
};

/*==============================================================================
 * Data Structs (Data-Driven)
 *==============================================================================*/

/// @brief Statistik dan parameter gameplay enemy, di-load dari JSON.
struct EnemyStats
{
    float maxHealth;             ///< Batas HP maksimum
    float speed;                 ///< Kecepatan gerak saat patroli/kembali ke spawn
    float chaseSpeed;            ///< Kecepatan gerak saat mengejar pemain
    float damage;                ///< Damage per serangan
    float baseDetectionRange;    ///< Jarak deteksi saat idle/patroli
    float chaseDetectionRange;   ///< Jarak deteksi saat mengejar (lebih sulit kabur)
    float attackRange;           ///< Jarak minimum untuk memicu serangan
    float healthRegenRate;       ///< HP yang pulih per detik saat di luar pertempuran
    float healthRegenDelay;      ///< Jeda (detik) setelah terkena damage sebelum regen aktif
    float patrolRadius;          ///< Radius maksimum patroli dari titik spawn
    float turnBaseTriggerChance; ///< Probabilitas memicu combat turn-based (0.0 - 1.0)
    bool canTriggerTurnBased;    ///< Apakah enemy ini eligible memicu combat turn-based
};

/// @brief Ukuran dan offset hitbox enemy, di-load dari JSON.
struct EnemyHitboxData
{
    Vector2 size;   ///< Lebar dan tinggi hitbox {width, height}
    Vector2 offset; ///< Offset hitbox relatif terhadap Position {offsetX, offsetY}
};

/// @brief Single source of truth untuk satu tipe enemy, di-load dari JSON.
struct EnemyDefinition
{
    int id;           ///< ID unik, digunakan sebagai key lookup
    std::string name; ///< Nama tipe enemy (e.g. "Slime", "Skeleton")
    EnemyRank rank;
    EnemyStats stats;            ///< Statistik gameplay
    EnemyHitboxData hitbox;      ///< Konfigurasi hitbox
    const AnimationSet *animSet; ///< Pointer ke AnimationSet global, di-resolve dari type
};

// data driven management class
class EnemyDataManager
{
public:
    void Load(const std::string &path);
    const EnemyDefinition &Get(const std::string &name) const;
    std::vector<std::string> GetAllNames() const;

private:
    std::unordered_map<std::string, EnemyDefinition> definitions_;
};

const AnimationSet *ResolveAnimSet(const std::string &name);

/*==============================================================================
 * Enemy Class
 *==============================================================================*/

/// @brief Kelas musuh yang mengimplementasikan logika AI berbasis FSM.
/// Mewarisi dari Entity untuk properti dasar (Position, Health, dll).
class Enemy : public Entity
{
public:
    Enemy();
    virtual ~Enemy();

    /// @brief Inisialisasi enemy pada posisi dunia tertentu dari EnemyDefinition.
    /// @param pos Posisi spawn dalam world space
    /// @param name Nama instance enemy
    /// @param mapId ID object Tiled untuk persistensi kematian
    /// @param def Definisi tipe enemy dari EnemyDataManager
    void Init(Vector2 pos, const char *name, int mapId, const EnemyDefinition &def);

    void Update() override;
    void Render() override;
    void TakeDamage(float amount, Vector2 knockback = {0, 0}) override;

    void UpdateAI();       ///< Titik masuk utama logika AI, dipanggil tiap frame
    bool CheckPlayerLoS(); ///< Cek Line of Sight ke pemain via raycasting

    // --- Definisi ---
    const EnemyDefinition *Def = nullptr; ///< Pointer ke definisi tipe, di-assign saat Init
    EnemyDefinition DefStorage;           ///< Copy definisi agar pointer tetap valid selama enemy hidup

    // --- Identitas ---
    std::string Name;     ///< Nama instance enemy
    int MapObjectID = -1; ///< ID object Tiled untuk persistensi kematian
    EnemyRank rank;

    // --- AI State ---
    EnemyAIState AIState = ENEMY_IDLE; ///< State FSM aktif saat ini
    float DetectionRange;              ///< Jarak deteksi aktif, berubah sesuai AIState (runtime)

    // --- Runtime Stats ---
    float HealthRegenTimer; ///< Countdown sebelum regen aktif, reset saat terkena damage (runtime)

    // --- Turn-Based ---
    bool isTurnBasedMode = false; ///< True jika sedang dalam mode combat turn-based
    bool isMyTurn = false;        ///< True jika giliran enemy di mode turn-based

    // --- Animasi ---
    Animation Anim;              ///< State animasi aktif (runtime)
    const AnimationSet *AnimSet; ///< Pointer ke AnimationSet aktif, di-resolve dari Def->type

    // --- Patroli ---
    Vector2 PatrolTarget;              ///< Titik tujuan patroli saat ini (runtime)
    Vector2 SpawnPoint;                ///< Titik spawn awal, pusat area patroli
    Rectangle SpawnRect;               // valid kalau spawn dari rect, otherwise {0,0,0,0}
    float PatrolTimer;                 ///< Timer tunggu di titik patroli (runtime)
    const float PatrolWaitTime = 2.0f; ///< Durasi tunggu sebelum patroli ke titik berikutnya

    // --- Hitbox ---
    float HitboxWidth;
    float HitboxHeight;
    float HitboxOffsetX;
    float HitboxOffsetY;
    Rectangle GetHitbox() const override
    {
        return {Position.x + HitboxOffsetX, Position.y + HitboxOffsetY, HitboxWidth, HitboxHeight};
    }

    EnemySteering Steering;

    // getter function
    Vector2 GetVelocity() { return Velocity; }

    RayHitResult CastDebugRay(Vector2 dir, float maxDist, std::vector<MapObject> &obstacles,
                              RayCastMode mode, float halfAngleDeg, int rayCount)
    {
        if (mode == RayCastMode::CONE)
            return Ray.CastCone(GetCenter(), dir, maxDist, halfAngleDeg, rayCount, obstacles);
        return Ray.Cast(GetCenter(), dir, maxDist, obstacles);
    }
    float GetRayLength() { return rayLength; }
    float GetRayDetectionLength() { return rayDetectionLength; }
    float GetHitboxValue() { return HitBoxValue; }
    float GetOffSetValue() { return OffSetValue; }
    float GetTileCenterOffset() { return TileCenterOffset; }

    const FlowField *GetReturnFlowField() const { return ReturnFlowField; }
    void SetReturnFlowField(FlowField *ff) { ReturnFlowField = ff; }

    SteeringContext BuildSteeringContext() const
    {
        SteeringContext ctx;
        ctx.Position = Position;
        ctx.Velocity = Velocity;
        ctx.HitBoxValue = HitBoxValue;
        ctx.OffsetValue = OffSetValue;
        ctx.TileCenterOffset = TileCenterOffset;
        ctx.DetectionRange = DetectionRange;
        ctx.rayLength = rayLength;
        ctx.rayDetectionLength = rayDetectionLength;
        ctx.PlayerCenter = PlayerInstance.GetCenter();
        ctx.PlayerHitbox = PlayerInstance.GetHitbox();
        ctx.SpawnPoint = SpawnPoint;
        ctx.ReturnFlowField = ReturnFlowField;
        return ctx;
    }

private:
    void HandleIdle();
    void HandlePatrol();
    void HandleChase();
    void HandleAttack();
    void HandleReturn();
    void PerformAttack();

    FlowField *ReturnFlowField = nullptr;

    // buat handle logic ai path finding
    Vector2 Velocity = {0, 0};                 // arah gerak frame sebelumnya (dinormalisasi)
    RayCast Ray;                               ///< Digunakan untuk pemeriksaan LoS dan deteksi obstacle
    float TileCenterOffset = TILE_SIZE * 0.5f; // offset untuk ai pathfinding
    float HitBoxValue = 24.0f;                 // hitbox untuk ai pathfinding
    float OffSetValue = 0.0f;                  // offset untuk ai path finding
    float rayLength = TILE_SIZE * 2.0f;        // panjang raycast untuk ai path finding
    float rayDetectionLength = TILE_SIZE * 2.1f;
    float ReturnScanTimer = 0.f;

    float AttackCooldownTimer;         ///< Sisa waktu cooldown serangan (runtime)
    const float AttackCooldown = 1.0f; ///< Durasi cooldown antar serangan
    bool PlayerWasInRange = false;     ///< Flag mencegah serangan ganda dalam satu frame

    // --- Feedback Visual & Kematian ---
    float HitFlashTimer;              ///< Timer tint merah saat terkena damage (runtime)
    Vector2 KnockbackVelocity;        ///< Vektor knockback aktif (runtime)
    float DeathTimer;                 ///< Timer animasi kematian (runtime)
    const float DeathDuration = 1.2f; ///< Durasi animasi kematian sebelum di-deactivate

    void MoveTowards(Vector2 target, float speed); ///< Helper gerak ke target dengan collision check
};

/*==============================================================================
 * Utility Functions
 *==============================================================================*/
EnemyRank ParseRank(const std::string &s);
std::vector<std::string> GetNamesByRank(EnemyRank rank);

// Spawn functions
void SpawnEnemiesFromMap();
void SpawnAtPoint(const tson::Object &obj, EnemyRank rank);
void SpawnInRect(const tson::Object &obj, EnemyRank rank);
void SpawnBoss(const tson::Object &obj);
void InitEnemy();

void SaveEnemiesForMap(const std::string &mapPath);
bool LoadEnemiesForMap(const std::string &mapPath);
void ClearEnemies();

// intance global enemy
extern EnemyDataManager enemyData;
