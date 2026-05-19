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
constexpr int SPAWN_PINPOINT_NORMAL_MIN = 9;
constexpr int SPAWN_PINPOINT_NORMAL_MAX = 13;
constexpr int SPAWN_PINPOINT_ELITE_MIN = 3;
constexpr int SPAWN_PINPOINT_ELITE_MAX = 7;
constexpr int SPAWN_RECT_NORMAL_MIN = 20;
constexpr int SPAWN_RECT_NORMAL_MAX = 25;
constexpr int SPAWN_RECT_ELITE_MIN = 10;
constexpr int SPAWN_RECT_ELITE_MAX = 15;
constexpr int SPAWN_RETRY_LIMIT = 200;

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
    LINE, // raycast satu garis lurus
    CONE  // raycast berbentuk cone untuk area deteksi lebih lebar
};

typedef enum EnemyRank
{
    ENEMY_NORMAL, // enemy biasa
    ENEMY_ELITE,  // enemy elite
    ENEMY_BOSS    // enemy boss
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
    int id;                      ///< ID unik, digunakan sebagai key lookup
    std::string name;            ///< Nama tipe enemy (e.g. "Slime", "Skeleton")
    EnemyRank rank;              ///< Rank enemy untuk filter spawn dan balancing
    EnemyStats stats;            ///< Statistik gameplay
    EnemyHitboxData hitbox;      ///< Konfigurasi hitbox
    const AnimationSet *animSet; ///< Pointer ke AnimationSet global, di-resolve dari type
};

// data driven management class
class EnemyDataManager
{
public:
    void Load(const std::string &path);                        // load seluruh definisi enemy dari file JSON
    const EnemyDefinition &Get(const std::string &name) const; // ambil definisi enemy berdasarkan nama
    std::vector<std::string> GetAllNames() const;              // ambil semua nama enemy yang sudah ter-load

private:
    std::unordered_map<std::string, EnemyDefinition> definitions_; // lookup definisi enemy berdasarkan nama
};

const AnimationSet *ResolveAnimSet(const std::string &name); // pilih AnimationSet berdasarkan nama enemy

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

    void Update() override;                                             // update state runtime enemy tiap frame
    void Render() override;                                             // gambar enemy, debug overlay, dan health bar bila perlu
    void TakeDamage(float amount, Vector2 knockback = {0, 0}) override; // terima damage dan efek knockback

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
    Rectangle SpawnRect;               // area spawn asal jika enemy dibuat dari rectangle spawn
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

    EnemySteering Steering; // helper steering untuk chase dan return pathfinding

    // getter function
    // ambil velocity enemy dari frame terakhir
    Vector2 GetVelocity() { return Velocity; }

    // cast ray debug dari pusat enemy dengan mode line atau cone
    RayHitResult CastDebugRay(Vector2 dir, float maxDist, std::vector<MapObject> &obstacles,
                              RayCastMode mode, float halfAngleDeg, int rayCount)
    {
        if (mode == RayCastMode::CONE)
            return Ray.CastCone(GetCenter(), dir, maxDist, halfAngleDeg, rayCount, obstacles);
        return Ray.Cast(GetCenter(), dir, maxDist, obstacles);
    }
    float GetRayLength() { return rayLength; }                   // ambil panjang raycast steering
    float GetRayDetectionLength() { return rayDetectionLength; } // ambil radius deteksi langsung steering
    float GetHitboxValue() { return HitBoxValue; }               // ambil ukuran hitbox untuk validasi pathfinding
    float GetOffSetValue() { return OffSetValue; }               // ambil offset hitbox untuk validasi pathfinding
    float GetTileCenterOffset() { return TileCenterOffset; }     // ambil offset pusat tile

    const FlowField *GetReturnFlowField() const { return ReturnFlowField; } // ambil flow field return aktif
    void SetReturnFlowField(FlowField *ff) { ReturnFlowField = ff; }        // set flow field untuk kembali ke spawn

    // bangun konteks steering dari state runtime enemy dan player saat ini
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

    Vector2 SeparationForce = {0, 0};

private:
    void HandleIdle();    // jalankan state idle
    void HandlePatrol();  // jalankan state patrol
    void HandleChase();   // jalankan state chase
    void HandleAttack();  // jalankan state attack
    void HandleReturn();  // jalankan state return
    void PerformAttack(); // eksekusi damage ke player jika tidak terhalang obstacle

    FlowField *ReturnFlowField = nullptr;

    // buat handle logic ai path finding
    Vector2 Velocity = {0, 0};                   // arah gerak frame sebelumnya (dinormalisasi)
    RayCast Ray;                                 ///< Digunakan untuk pemeriksaan LoS dan deteksi obstacle
    float TileCenterOffset = FRAME_SIZE * 0.5f;   // offset untuk ai pathfinding
    float HitBoxValue = 24.0f;                   // hitbox untuk ai pathfinding
    float OffSetValue = 0.0f;                    // offset untuk ai path finding
    float rayLength = FRAME_SIZE * 2.0f;          // panjang raycast untuk ai path finding
    float rayDetectionLength = FRAME_SIZE * 2.1f; // radius deteksi langsung ke player
    float ReturnScanTimer = 0.f;                 // timer pencarian ulang return flow field

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
EnemyRank ParseRank(const std::string &s);               // konversi string rank dari JSON ke enum EnemyRank
std::vector<std::string> GetNamesByRank(EnemyRank rank); // ambil nama enemy yang cocok dengan rank tertentu

// Spawn functions
void SpawnEnemiesFromMap();                                                        // spawn enemy dari object spawn di map aktif
void SpawnAtPoint(const MapObject *obj, EnemyRank rank);                           // spawn satu enemy di titik object berdasarkan rank
void SpawnInRect(const MapObject *obj, const std::string &enemyName, float ratio); // spawn beberapa enemy acak di area rectangle
void SpawnBoss(const MapObject *obj);                                              // spawn satu boss dari object spawn
void InitEnemy();                                                                  // load texture dan data enemy

void SaveEnemiesForMap(const std::string &mapPath);
bool LoadEnemiesForMap(const std::string &mapPath);
void ClearEnemies(); // hapus semua enemy aktif dari entity manager

// intance global enemy
extern EnemyDataManager enemyData;
