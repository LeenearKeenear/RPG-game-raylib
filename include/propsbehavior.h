#pragma once

/**
 * @file propsbehavior.h
 * @brief Props & Trap Behavior System
 *
 * Header ini mendeklarasikan semua manager untuk interactable props dan trap:
 * - ChestManager: chest yang bisa dibuka player untuk loot item
 * - SpikeManager: trap spike dengan timer aktif/nonaktif dan damage area
 * - BombManager: trap bomb yang meledak saat dipukul, dengan chain reaction
 */

#include "map.h"
#include "mapLogic.h"
#include "animation.h"
#include "player.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include <random>
#include <unordered_set>
#include <string>

/*==============================================================================
 * ObjectState Enum
 *==============================================================================*/

/**
 * @brief State universal untuk semua tile object
 */
enum class ObjectState
{
    Closed,  // Chest belum dibuka
    Open,    // Chest sudah dibuka
    Active,  // Trap sedang aktif
    Inactive // Trap sedang tidak aktif
};

/*==============================================================================
 * TileObject Struct
 *==============================================================================*/

/**
 * @brief Representasi universal untuk semua props dan trap di map
 */
struct TileObject
{
    Vector2 position;  // Posisi final setelah snap ke tile grid
    Rectangle bounds;  // Bounding box asli dari MapObject Tiled
    ObjectState state; // State saat ini
    std::string name;  // Nama object dari Tiled untuk identifikasi
};

/*==============================================================================
 * SpawnObject
 *==============================================================================*/

/**
 * @brief Spawn semua object dari Tiled ke manager masing-masing
 *
 * Entry point yang dipanggil saat map selesai di-load.
 */
void SpawnObject(void);

// helper encode data string
inline std::string EncodePos(Vector2 pos)
{
    return std::to_string((int)pos.x) + "_" + std::to_string((int)pos.y);
}

/*==============================================================================
 * ChestManager
 *==============================================================================*/

/**
 * @brief Manager untuk semua chest di map
 *
 * Menangani spawn, interaksi player, dan distribusi loot.
 */
class ChestManager
{
public:
    /** @brief Spawn semua chest dari object layer Tiled */
    void SpawnChests(const std::vector<MapObject *> &chestObjects);

    /**
     * @brief Trigger interaksi player dengan chest terdekat
     * @param hitPos Posisi interaksi player
     */
    void Interact(Vector2 hitPos);

    /** @brief Render semua chest (placeholder sprite) */
    void Render();

    /** @brief Bersihkan semua data chest */
    void Clear();

private:
    std::vector<TileObject> chests;
    std::unordered_set<std::string> consumedPositions;

    /**
     * @brief Cari chest terdekat dari titik hit
     * @param hitPos Posisi hit
     * @param threshold Toleransi jarak ke tepi bounds
     * @return Pointer ke chest terdekat, nullptr jika tidak ada
     */
    TileObject *FindChest(Vector2 hitPos, float threshold = 32.0f);

    /**
     * @brief Spawn loot item secara random di sekitar chest
     * @param chest Chest yang baru dibuka
     */
    void TriggerLoot(TileObject &chest);
};

/*==============================================================================
 * SpikeManager
 *==============================================================================*/

using SpikeCallback = std::function<void(TileObject &)>;

/**
 * @brief Manager untuk semua trap spike di map
 *
 * Tiap spike punya durasi aktif/nonaktif yang di-randomisasi saat spawn.
 * Damage ke player dan enemy menggunakan cooldown global.
 */
class SpikeManager
{
public:
    /** @brief Spawn semua spike dari object layer Tiled */
    void SpawnSpikes(const std::vector<MapObject *> &spikeObjects);

    /**
     * @brief Update timer dan damage spike tiap frame
     * @param deltaTime Waktu antar frame
     * @param playerBounds Bounding box player
     * @param player Pointer ke player untuk apply damage
     */
    void Update(float deltaTime, Rectangle playerBounds, Player *player);

    /** @brief Render semua spike (placeholder sprite) */
    void Render();

    /** @brief Bersihkan semua data spike */
    void Clear();

private:
    /**
     * @brief Data internal satu spike
     */
    struct SpikeData
    {
        TileObject tile;
        float activeTimer;      // Sisa waktu aktif
        float inactiveTimer;    // Sisa waktu nonaktif
        float activeDuration;   // Total durasi aktif (randomized)
        float inactiveDuration; // Total durasi nonaktif (randomized)
        float damageCooldown;   // Cooldown damage per spike (unused, pakai global)
        SpikeCallback onActivate;
        SpikeCallback onDeactivate;
        SpikeCallback onDamagePlayer;
    };

    // Konstanta timing dan damage spike
    static constexpr float SPIKE_ACTIVE_MAX = 6.0f;      // Durasi aktif maksimum (detik)
    static constexpr float SPIKE_ACTIVE_MIN = 3.0f;      // Durasi aktif minimum (detik)
    static constexpr float SPIKE_INACTIVE_MAX = 7.0f;    // Durasi nonaktif maksimum (detik)
    static constexpr float SPIKE_INACTIVE_MIN = 4.0f;    // Durasi nonaktif minimum (detik)
    static constexpr float SPIKE_DAMAGE = 10.0f;         // Damage per hit
    static constexpr float SPIKE_DAMAGE_COOLDOWN = 1.0f; // Cooldown damage global (detik)

    float globalPlayerDamageCooldown = 0.0f; // Cooldown damage ke player (shared semua spike)
    float globalEnemyDamageCooldown = 0.0f;  // Cooldown damage ke enemy (shared semua spike)

    std::vector<SpikeData> spikes;

    /**
     * @brief Generate seed dari nama object untuk randomisasi timer
     * @param name Nama object spike dari Tiled
     * @return Seed hasil hash nama
     */
    unsigned int SeedFromName(const std::string &name);

    /**
     * @brief Setup callback onActivate, onDeactivate, onDamagePlayer
     * @param spike SpikeData target
     */
    void SetupCallbacks(SpikeData &spike);
};

/*==============================================================================
 * BombManager
 *==============================================================================*/

using BombCallback = std::function<void(TileObject &)>;
using BombExplodeCallback = std::function<void(TileObject &, float)>;

/**
 * @brief Manager untuk semua trap bomb di map
 *
 * Bomb meledak saat terkena serangan player. Ledakan mengenai player,
 * enemy, dan bomb lain dalam radius (chain reaction).
 */
class BombManager
{
public:
    /** @brief Spawn semua bomb dari object layer Tiled */
    void SpawnBombs(const std::vector<MapObject *> &bombObjects);

    /**
     * @brief Update state semua bomb tiap frame
     * @param deltaTime Waktu antar frame
     * @param playerBounds Bounding box player
     * @param player Pointer ke player
     */
    void Update(float deltaTime, Rectangle playerBounds, Player *player);

    /** @brief Render semua bomb (placeholder sprite) */
    void Render();

    /** @brief Bersihkan semua data bomb */
    void Clear();

    /** @brief Spawn ulang semua bomb dari spawnPoints (debug only, disabled) */
    void SpawnAll();

    /**
     * @brief Cari bomb terdekat dari titik hit
     * @param hitPos Posisi hit
     * @param threshold Toleransi jarak ke tepi bounds
     * @return Pointer ke TileObject bomb terdekat, nullptr jika tidak ada
     */
    TileObject *FindBomb(Vector2 hitPos, float threshold = 32.0f);

    /**
     * @brief Trigger ledakan bomb yang terkena hitbox serangan player
     * @param attackHitbox Hitbox serangan player
     * @param playerBounds Bounding box player
     * @param player Pointer ke player
     */
    void HitByAttack(Rectangle attackHitbox, Rectangle playerBounds, Player *player);

private:
    /**
     * @brief Data internal satu bomb
     */
    struct BombData
    {
        TileObject tile;
        bool isAlive;             // False jika sudah selesai meledak
        bool isExploding;         // True selama animasi ledakan berlangsung
        bool isTriggered = false; // Guard untuk mencegah infinite loop chain reaction
        float explosionTimer;     // Sisa waktu animasi ledakan
        BombCallback onHit;
        BombExplodeCallback onExplode;
        BombCallback onDamagePlayer;
    };

    // Konstanta bomb
    static constexpr float BOMB_EXPLOSION_RADIUS = 80.0f;  // Radius area ledakan (pixel)
    static constexpr float BOMB_DAMAGE = 25.0f;            // Damage ledakan
    static constexpr float BOMB_EXPLOSION_DURATION = 0.3f; // Durasi animasi ledakan (detik)

    std::vector<BombData> bombs;
    std::unordered_set<std::string> consumedPositions;
    Player *playerRef = nullptr;

    /** @brief Setup callback onHit, onExplode, onDamagePlayer */
    void SetupCallbacks(BombData &bomb);

    /**
     * @brief Trigger ledakan bomb, damage area, dan chain reaction
     * @param bomb BombData yang diledakkan
     * @param playerBounds Bounding box player
     * @param player Pointer ke player
     */
    void Explode(BombData &bomb, Rectangle playerBounds, Player *player);

    /**
     * @brief Cek apakah target berada dalam radius ledakan
     *
     * Pakai nearest-point check, bukan center-to-center.
     *
     * @param bombPos Posisi center bomb
     * @param target Bounding box target
     * @return true jika jarak nearest point <= BOMB_EXPLOSION_RADIUS
     */
    bool IsInExplosionRadius(Vector2 bombPos, Rectangle target);
};

/*==============================================================================
 * Global Manager Instances
 *==============================================================================*/

extern ChestManager chestManager;
extern SpikeManager spikeManager;
extern BombManager bombManager;