#pragma once

/**
 * @file propsbehavior.h
 * @brief Props & Trap Behavior System
 *
 * Header ini mendeklarasikan semua manager untuk interactable props dan trap:
 * - ChestManager: chest yang bisa dibuka player untuk loot item
 * - SpikeManager: trap spike dengan timer aktif/nonaktif dan damage area
 * - BombManager: trap bomb yang meledak saat dipukul, dengan chain reaction
 * - CrateManager: crate yang bisa dihancurkan player/bomb dan punya chance drop loot
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
    std::string uuid;  ///< Unique identifier for persistent entity matching across save/load cycles. Generated at spawn time, persisted in save files, used for restore matching.
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

/**
 * @brief Trigger hit attack player ke props yang bisa dihancurkan.
 * @param attackHitbox Hitbox serangan player
 * @param playerBounds Bounding box player untuk efek props tertentu
 * @param player Pointer ke player
 */
void HitPropsByAttack(Rectangle attackHitbox, Rectangle playerBounds, Player *player);

/**
 * @brief Encode posisi world space menjadi key string untuk tracking object.
 * @param pos Posisi object dalam world space
 * @return String key berbentuk "x_y"
 */
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
    int Render(Rectangle viewRect);

    /** @brief Bersihkan semua data chest */
    void Clear();

    /**
     * @brief Ambil jumlah chest yang sedang dikelola.
     * @return Jumlah chest aktif di manager
     */
    size_t GetCount() const { return chests.size(); }

    /** @brief Dapatkan posisi chest yang sudah dikonsumsi */
    const std::unordered_set<std::string>& GetConsumedPositions() const { return consumedPositions; }

    /** @brief Set posisi chest yang sudah dikonsumsi (untuk restore state) */
    void SetConsumedPositions(const std::unordered_set<std::string>& positions) { consumedPositions = positions; }

private:
    std::vector<TileObject> chests;                    // daftar chest yang sedang dikelola
    std::unordered_set<std::string> consumedPositions; // posisi chest yang sudah dikonsumsi agar tidak diproses ulang

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

using SpikeCallback = std::function<void(TileObject &)>; // callback untuk event spike yang menerima TileObject

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
    int Render(Rectangle viewRect);

    /** @brief Bersihkan semua data spike */
    void Clear();

    /**
     * @brief Ambil jumlah spike yang sedang dikelola.
     * @return Jumlah spike aktif di manager
     */
    size_t GetCount() const { return spikes.size(); }

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

    std::vector<SpikeData> spikes; // daftar spike yang sedang dikelola

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
     * @brief Trigger ledakan bomb yang terkena hitbox serangan player
     * @param attackHitbox Hitbox serangan player
     * @param playerBounds Bounding box player
     * @param player Pointer ke player
     */
    void HitByAttack(Rectangle attackHitbox, Rectangle playerBounds, Player *player);

    /**
     * @brief Update state semua bomb tiap frame
     * @param deltaTime Waktu antar frame
     * @param playerBounds Bounding box player
     * @param player Pointer ke player
     */
    void Update(float deltaTime, Rectangle playerBounds, Player *player);

    /** @brief Render semua bomb (placeholder sprite) */
    int Render(Rectangle viewRect);

    /** @brief Bersihkan semua data bomb */
    void Clear();

    /**
     * @brief Ambil jumlah bomb yang sedang dikelola.
     * @return Jumlah bomb aktif di manager
     */
    size_t GetCount() const { return bombs.size(); }

    /**
     * @brief Dapatkan posisi bomb yang sudah meledak (consumed).
     * @return Const reference ke unordered_set posisi yang sudah dikonsumsi
     */
    const std::unordered_set<std::string>& GetConsumedPositions() const { return consumedPositions; }

    /**
     * @brief Set posisi bomb yang sudah meledak (untuk restore save state).
     * @param positions Set posisi yang sudah dikonsumsi
     */
    void SetConsumedPositions(const std::unordered_set<std::string>& positions) { consumedPositions = positions; }

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
    };

    // Konstanta bomb
    static constexpr float BOMB_EXPLOSION_RADIUS = 80.0f;  // Radius area ledakan (pixel)
    static constexpr float BOMB_DAMAGE = 25.0f;            // Damage ledakan
    static constexpr float BOMB_EXPLOSION_DURATION = 0.6f; // Durasi animasi ledakan (detik)

    std::vector<BombData> bombs;                       // daftar bomb yang sedang dikelola
    std::unordered_set<std::string> consumedPositions; // posisi bomb yang sudah dikonsumsi agar tidak diproses ulang
    Player *playerRef = nullptr;                       // referensi player terakhir untuk kebutuhan update/interaction bomb

    /**
     * @brief Cari bomb terdekat dari titik hit
     * @param hitPos Posisi hit
     * @param threshold Toleransi jarak ke tepi bounds
     * @return Pointer ke TileObject bomb terdekat, nullptr jika tidak ada
     */
    TileObject *FindBomb(Vector2 hitPos, float threshold = 32.0f);

    /**
     * @brief Trigger ledakan bomb, damage area, dan chain reaction
     * @param bomb BombData yang diledakkan
     * @param playerBounds Bounding box player
     * @param player Pointer ke player
     */
    void Explode(BombData &bomb, Rectangle playerBounds, Player *player);
};

/*==============================================================================
 * CrateManager
 *==============================================================================*/

/**
 * @brief Manager untuk semua crate di map.
 *
 * Crate bisa dihancurkan oleh serangan player atau ledakan bomb,
 * lalu punya peluang menjatuhkan loot.
 */
class CrateManager
{
public:
    void SpawnCrates(const std::vector<MapObject *> &crateObjects); // spawn semua crate dari object layer Tiled
    void HitByAttack(Rectangle attackHitbox);                       // hancurkan crate yang terkena hitbox serangan player
    void Update();                                                  // hapus crate yang sudah tidak aktif dari daftar runtime
    void HitByExplosion(Vector2 bombPos, BombManager *bomber);      // hancurkan crate yang terkena radius ledakan bomb
    int Render(Rectangle viewRect);                                 // render crate yang terlihat dalam view
    void Clear();                                                   // bersihkan semua data crate

    /**
     * @brief Ambil jumlah crate yang sedang dikelola.
     * @return Jumlah crate aktif di manager
     */
    size_t GetCount() const { return crates.size(); }

    /**
     * @brief Dapatkan posisi crate yang sudah dihancurkan (consumed).
     * @return Const reference ke unordered_set posisi yang sudah dikonsumsi
     */
    const std::unordered_set<std::string>& GetConsumedPositions() const { return consumedPositions; }

    /**
     * @brief Set posisi crate yang sudah dihancurkan (untuk restore save state).
     * @param positions Set posisi yang sudah dikonsumsi
     */
    void SetConsumedPositions(const std::unordered_set<std::string>& positions) { consumedPositions = positions; }

private:
    struct CrateData
    {
        TileObject tile; // data object crate di map
        bool isAlive;    // false jika crate sudah dihancurkan
    };

    static constexpr float CRATE_LOOT_CHANCE = 0.10f; // 10% chance drop loot

    std::vector<CrateData> crates;                     // daftar crate yang sedang dikelola
    std::unordered_set<std::string> consumedPositions; // posisi crate yang sudah dihancurkan agar tidak spawn ulang

    TileObject *FindCrate(Vector2 hitPos, float threshold = 32.0f); // cari crate terdekat dari titik hit
    void Destroy(CrateData &crate);                                 // hancurkan crate, hapus obstacle, dan trigger loot
    void TriggerLoot(TileObject &crate);                            // roll peluang drop loot dari crate
};

/*==============================================================================
 * Global Manager Instances
 *==============================================================================*/

extern ChestManager chestManager; // instance global manager chest
extern SpikeManager spikeManager; // instance global manager spike
extern BombManager bombManager;   // instance global manager bomb
extern CrateManager crateManager; // instance global manager crate
