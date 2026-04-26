#pragma once

#include "map.h"
#include "mapLogic.h"
#include "animation.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include <random>

// enum buat object state
enum class ObjectState
{
    Closed,
    Open,
    Active,
    Inactive
};

// struct universal buat semua proeperti
struct TileObject
{
    Vector2 position; // posisi final (sudah dikoreksi ke tile grid)
    Rectangle bounds; // bounds asli dari MapObject
    ObjectState state;
    std::string name; // nama dari Tiled (buat identifikasi)
};

// entry buat nge spawn semua jenis tile
void SpawnObject(void);

// class buat chest
class ChestManager
{
public:
    void SpawnChests(const std::vector<MapObject *> &chestObjects);
    void Interact(Vector2 hitPos);
    void Render();
    void Clear();

private:
    std::vector<TileObject> chests;

    TileObject *FindChest(Vector2 hitPos, float threshold = 32.0f);
    void TriggerLoot(TileObject &chest);
};

// class buat trap spike
using SpikeCallback = std::function<void(TileObject &)>;

class SpikeManager
{
public:
    void SpawnSpikes(const std::vector<MapObject *> &spikeObjects);
    void Update(float deltaTime, Rectangle playerBounds);
    void Render();
    void Clear();

private:
    struct SpikeData
    {
        TileObject tile;
        float activeTimer;
        float inactiveTimer;
        float activeDuration;   // range: 1.0 - 3.0 detik (adjustable)
        float inactiveDuration; // range: 1.0 - 3.0 detik (adjustable)
        float damageCooldown;   // default: 0.5 detik (adjustable)
        SpikeCallback onActivate;
        SpikeCallback onDeactivate;
        SpikeCallback onDamagePlayer;
    };

// konstanta buat timing spike nya
#define SPIKE_ACTIVE_MAX 6.0f      // nilai maksimun pas spike aktif (adjustable)
#define SPIKE_ACTIVE_MIN 3.0f      // nilai minimum pas spike aktif (adjustable)
#define SPIKE_INACTIVE_MAX 7.0f    // nilai maksimun pas spike gak aktif (adjustable)
#define SPIKE_INACTIVE_MIN 4.0f    // nilai minimum pas spike gak aktif (adjustable)
#define SPIKE_DAMAGE_COOLDOWN 0.5f // adjustable

    std::vector<SpikeData> spikes;
    unsigned int SeedFromName(const std::string &name);
    void SetupCallbacks(SpikeData &spike);
};

// naming untuk tile
extern ChestManager chestManager;
extern SpikeManager spikeManager;