/**
 * @file propsbehavior.cpp
 * @brief Implementasi Props & Trap Behavior System
 *
 * File ini berisi implementasi untuk semua interactable props dan trap di dungeon:
 * - ChestManager: spawn, interaksi, dan loot chest
 * - SpikeManager: spawn, update timer aktif/nonaktif, damage player & enemy
 * - BombManager: spawn, explode, chain reaction, damage player & enemy
 *
 * Semua manager di-spawn via SpawnObject() yang dipanggil saat map di-load.
 */

#include "propsbehavior.h"
#include "item.h"
#include "enemy.h"
#include "enemy_ai.h"
#include "entities.h"

/*==============================================================================
 * Utility Functions
 *==============================================================================*/

/**
 * @brief Snap posisi ke grid tile terdekat
 * @param rawPos Posisi mentah dari Tiled
 * @return Posisi yang sudah di-snap ke kelipatan TILE_SIZE
 */
Vector2 SnapToTileGrid(Vector2 rawPos)
{
    return {
        std::floor(rawPos.x / TILE_SIZE) * TILE_SIZE,
        std::floor(rawPos.y / TILE_SIZE) * TILE_SIZE};
}

/**
 * @brief Cek apakah titik hit berada dalam bounds object (dengan toleransi)
 * @param hitPos Posisi hit
 * @param bounds Bounding box object
 * @param threshold Toleransi expand ke semua sisi
 * @param outCenter Output posisi center bounds (opsional)
 * @return true jika hitPos masuk dalam expanded bounds
 */
bool IsHitInBounds(Vector2 hitPos, Rectangle bounds, float threshold, Vector2 *outCenter = nullptr)
{
    Rectangle expanded = {
        bounds.x - threshold,
        bounds.y - threshold,
        bounds.width + threshold * 2,
        bounds.height + threshold * 2};

    if (outCenter)
    {
        outCenter->x = bounds.x + bounds.width / 2;
        outCenter->y = bounds.y + bounds.height / 2;
    }

    return CheckCollisionPointRec(hitPos, expanded);
}

/**
 * @brief Hitung jarak dari titik hit ke center bounds
 * @param hitPos Posisi hit
 * @param bounds Bounding box object
 * @return Jarak ke center
 */
float DistToCenter(Vector2 hitPos, Rectangle bounds)
{
    Vector2 center = {bounds.x + bounds.width / 2, bounds.y + bounds.height / 2};
    return Vector2Distance(hitPos, center);
}

/**
 * @brief Spawn semua object dari Tiled ke manager masing-masing
 *
 * Dipanggil sekali saat map selesai di-load. Mengambil object
 * berdasarkan type dari object layer dan mendistribusikannya
 * ke ChestManager, SpikeManager, dan BombManager.
 */
void SpawnObject()
{
    auto chestObjs = TiledHelper::GetObjectsByType(CHEST_TYPE_OBJECT_NAME);
    chestManager.SpawnChests(chestObjs);

    auto spikeObjs = TiledHelper::GetObjectsByType(SPIKE_TYPE_OBJECT_NAME);
    spikeManager.SpawnSpikes(spikeObjs);

    auto bombObjs = TiledHelper::GetObjectsByType(BOMB_TYPE_OBJECT_NAME);
    bombManager.SpawnBombs(bombObjs);
}

/*==============================================================================
 * ChestManager Implementation
 *==============================================================================*/

ChestManager chestManager;

/**
 * @brief Spawn semua chest dari object layer Tiled
 *
 * Snap posisi ke tile grid, set state awal Closed.
 *
 * @param chestObjects Daftar pointer MapObject bertipe chest
 */
void ChestManager::SpawnChests(const std::vector<MapObject *> &chestObjects)
{
    TraceLog(LOG_INFO, "SpawnChests called, consumed size: %d", (int)consumedPositions.size());
    chests.clear();
    for (auto *obj : chestObjects)
    {
        Vector2 snapped = SnapToTileGrid({obj->bounds.x, obj->bounds.y});

        TileObject c;
        c.name = obj->name;
        c.bounds = obj->bounds;
        c.position = snapped;

        if (consumedPositions.count(EncodePos(snapped)))
            c.state = ObjectState::Open; // udah pernah dibuka
        else
            c.state = ObjectState::Closed;

        chests.push_back(c);
    }
}

/**
 * @brief Cari chest terdekat dari titik hit
 *
 * Menggunakan expanded bounds agar titik di tepi tetap terdeteksi.
 *
 * @param hitPos Posisi hit dari player
 * @param threshold Toleransi jarak ke tepi bounds
 * @return Pointer ke chest terdekat, nullptr jika tidak ada
 */
TileObject *ChestManager::FindChest(Vector2 hitPos, float threshold)
{
    TileObject *closest = nullptr;
    float minDist = threshold;

    for (auto &chest : chests)
    {
        if (IsHitInBounds(hitPos, chest.bounds, threshold))
        {
            float dist = DistToCenter(hitPos, chest.bounds);
            if (dist < minDist)
            {
                minDist = dist;
                closest = &chest;
            }
        }
    }
    return closest; // nullptr kalau gak ada dalam threshold
}

/**
 * @brief Trigger interaksi player dengan chest
 *
 * Cari chest di sekitar hitPos, buka jika masih Closed, lalu trigger loot.
 *
 * @param hitPos Posisi interaksi player
 */
void ChestManager::Interact(Vector2 hitPos)
{
    TraceLog(LOG_INFO, "Looking for chest at (%.1f, %.1f), total chests: %d", hitPos.x, hitPos.y, (int)chests.size());
    TileObject *chest = FindChest(hitPos);
    if (!chest || chest->state == ObjectState::Open)
        return;
    chest->state = ObjectState::Open;
    consumedPositions.insert(EncodePos(chest->position));
    TriggerLoot(*chest);
}

/**
 * @brief Spawn loot item secara random di sekitar chest yang dibuka
 *
 * Jumlah item 1-3, posisi di-offset random di sekitar chest.
 * Rarity system belum diimplementasi.
 *
 * @param chest TileObject chest yang baru dibuka
 */
void ChestManager::TriggerLoot(TileObject &chest)
{
    // placeholder, rarity system nyusul
    TraceLog(LOG_INFO, "Chest opened at (%.1f, %.1f)", chest.position.x, chest.position.y);

    int jumlahLoot = GetRandomValue(1, 3);

    for (int i = 0; i < jumlahLoot; i++)
    {
        // Kasih sedikit offset random (misal sejauh -20 sampai 20 pixel)
        // Biar itemnya mencar di sekitar chest
        Vector2 spawnPos = {
            chest.position.x + (float)GetRandomValue(-60, 60),
            chest.position.y + (float)GetRandomValue(-60, 60)};

        itemData.SpawnItemAtLocation(spawnPos);
    }
}

/**
 * @brief Render semua chest ke layar
 *
 * State Closed = BROWN, Open = WHITE. Placeholder, belum pakai sprite.
 */
int ChestManager::Render(Rectangle viewRect)
{
    int rendered = 0;
    for (auto &c : chests)
    {
        if (!CheckCollisionRecs(c.bounds, viewRect))
            continue;
        rendered++;

        if (c.state == ObjectState::Closed)
            DrawRectangleRec(c.bounds, BROWN); // placeholder harus diganti pake skin
        else
            DrawRectangleRec(c.bounds, WHITE); // placeholder harus diganti pake skin
    }
    return rendered;
}

/**
 * @brief Bersihkan semua data chest
 */
void ChestManager::Clear()
{
    chests.clear();
}

/*==============================================================================
 * SpikeManager Implementation
 *==============================================================================*/

SpikeManager spikeManager;

/**
 * @brief Generate seed dari nama object untuk randomisasi timer
 *
 * Spike dengan nama sama akan punya seed yang sama,
 * dikombinasikan dengan global time seed agar tetap bervariasi tiap run.
 *
 * @param name Nama object spike dari Tiled
 * @return Seed unsigned int hasil hash nama
 */

unsigned int SpikeManager::SeedFromName(const std::string &name)
{
    unsigned int seed = 0;
    for (char c : name)
        seed = seed * 31 + static_cast<unsigned int>(c);
    return seed;
}

/**
 * @brief Spawn semua spike dari object layer Tiled
 *
 * Tiap spike dapat durasi aktif/nonaktif yang di-randomisasi
 * menggunakan kombinasi global time seed dan name seed.
 * State awal: Inactive.
 *
 * @param spikeObjects Daftar pointer MapObject bertipe spike
 */
void SpikeManager::SpawnSpikes(const std::vector<MapObject *> &spikeObjects)
{
    spikes.clear();
    for (auto *obj : spikeObjects)
    {
        SpikeData data;
        data.tile.name = obj->name;
        data.tile.bounds = obj->bounds;
        data.tile.state = ObjectState::Inactive;
        data.tile.position = SnapToTileGrid({obj->bounds.x, obj->bounds.y});

        unsigned int globalSeed = static_cast<unsigned int>(time(nullptr));
        unsigned int nameSeed = SeedFromName(obj->name);

        std::mt19937 rng(globalSeed ^ nameSeed);
        std::uniform_real_distribution<float> activeDist(SPIKE_ACTIVE_MIN, SPIKE_ACTIVE_MAX);
        std::uniform_real_distribution<float> inactiveDist(SPIKE_INACTIVE_MIN, SPIKE_INACTIVE_MAX);

        data.activeDuration = activeDist(rng);
        data.inactiveDuration = inactiveDist(rng);
        data.activeTimer = 0.0f;
        data.inactiveTimer = data.inactiveDuration;
        data.damageCooldown = 0.0f;

        SetupCallbacks(data);
        spikes.push_back(data);
    }
}

/**
 * @brief Setup callback untuk event spike
 *
 * - onActivate: set state ke Active
 * - onDeactivate: set state ke Inactive
 * - onDamagePlayer: placeholder log damage
 *
 * @param spike SpikeData yang akan di-setup callbacknya
 */
void SpikeManager::SetupCallbacks(SpikeData &spike)
{
    spike.onActivate = [](TileObject &tile)
    {
        tile.state = ObjectState::Active;
        // TraceLog(LOG_INFO, "Spike '%s' activated", tile.name.c_str());
    };

    spike.onDeactivate = [](TileObject &tile)
    {
        tile.state = ObjectState::Inactive;
        // TraceLog(LOG_INFO, "Spike '%s' deactivated", tile.name.c_str());
    };

    spike.onDamagePlayer = [](TileObject &tile)
    {
        // placeholder, sambungin ke player health system nanti
        TraceLog(LOG_INFO, "Spike '%s' damaged player", tile.name.c_str());
    };
}

/**
 * @brief Update timer dan damage spike tiap frame
 *
 * Alur per spike:
 * - Inactive: countdown inactiveTimer, switch ke Active jika habis
 * - Active: countdown activeTimer, switch ke Inactive jika habis
 * - Active: cek collision dengan player dan enemy, apply damage dengan cooldown global
 *
 * @param deltaTime Waktu antar frame
 * @param playerBounds Bounding box player untuk cek collision
 * @param player Pointer ke player untuk apply damage
 */
void SpikeManager::Update(float deltaTime, Rectangle playerBounds, Player *player)
{
    globalPlayerDamageCooldown -= deltaTime;
    globalEnemyDamageCooldown -= deltaTime;

    for (auto &spike : spikes)
    {
        if (spike.tile.state == ObjectState::Inactive)
        {
            spike.inactiveTimer -= deltaTime;
            if (spike.inactiveTimer <= 0.0f)
            {
                spike.activeTimer = spike.activeDuration;
                spike.inactiveTimer = 0.0f;
                spike.onActivate(spike.tile);
            }
            continue; // skip sisanya
        }

        // dari sini udah pasti Active
        spike.activeTimer -= deltaTime;
        if (spike.activeTimer <= 0.0f)
        {
            spike.inactiveTimer = spike.inactiveDuration;
            spike.activeTimer = 0.0f;
            spike.onDeactivate(spike.tile);
            continue;
        }
        // damage player
        if (CheckCollisionRecs(playerBounds, spike.tile.bounds) && globalPlayerDamageCooldown <= 0.0f && player)
        {
            globalPlayerDamageCooldown = SPIKE_DAMAGE_COOLDOWN;
            player->TakeDamage(SPIKE_DAMAGE);
        }

        // damage enemy
        if (globalEnemyDamageCooldown <= 0.0f)
        {
            for (auto entity : Entities::GetRegistry())
            {
                Enemy *enemy = dynamic_cast<Enemy *>(entity);
                if (!enemy || !enemy->IsActive || enemy->Health <= 0)
                    continue;
                if (CheckCollisionRecs(spike.tile.bounds, enemy->GetHitbox()))
                {
                    globalEnemyDamageCooldown = SPIKE_DAMAGE_COOLDOWN;
                    enemy->TakeDamage(SPIKE_DAMAGE, {0, 0});
                }
            }
        }
    }
}

/**
 * @brief Render semua spike ke layar
 *
 * Active = RED, Inactive = GRAY. Placeholder, belum pakai sprite.
 */
int SpikeManager::Render(Rectangle viewRect)
{
    int rendered = 0;
    for (auto &spike : spikes)
    {
        if (!CheckCollisionRecs(spike.tile.bounds, viewRect))
            continue;
        rendered++;

        if (spike.tile.state == ObjectState::Active)
            DrawRectangleRec(spike.tile.bounds, RED); // placeholder harus diganti pake skin
        else
            DrawRectangleRec(spike.tile.bounds, GRAY); // placeholder harus diganti pake skin
    }
    return rendered;
}

/**
 * @brief Bersihkan semua data spike
 */
void SpikeManager::Clear()
{
    spikes.clear();
}

/*==============================================================================
 * BombManager Implementation
 *==============================================================================*/

BombManager bombManager;

/**
 * @brief Spawn semua bomb dari object layer Tiled
 *
 * Snap posisi ke tile grid, set state awal Active,
 * dan daftarkan ke DynamicObstacles untuk pathfinding enemy.
 *
 * @param bombObjects Daftar pointer MapObject bertipe bomb
 */
void BombManager::SpawnBombs(const std::vector<MapObject *> &bombObjects)
{
    bombs.clear();
    for (auto *obj : bombObjects)
    {
        Vector2 snapped = SnapToTileGrid({obj->bounds.x, obj->bounds.y});
        if (consumedPositions.count(EncodePos(snapped)))
            continue; // skip yang udah meledak

        BombData data;
        data.tile.name = obj->name;
        data.tile.bounds = obj->bounds;
        data.tile.state = ObjectState::Active;
        data.tile.position = SnapToTileGrid({obj->bounds.x, obj->bounds.y});

        data.isAlive = true;
        data.isExploding = false;
        data.explosionTimer = 0.0f;

        SetupCallbacks(data);
        bombs.push_back(data);
        DynamicObstacles.push_back(data.tile.bounds);
    }
}

/**
 * @brief Setup callback untuk event bomb
 *
 * - onHit: log saat bomb kena serangan
 * - onExplode: log saat bomb meledak
 * - onDamagePlayer: log saat bomb damage player
 *
 * @param bomb BombData yang akan di-setup callbacknya
 */
void BombManager::SetupCallbacks(BombData &bomb)
{
    bomb.onHit = [](TileObject &tile)
    {
        TraceLog(LOG_INFO, "Bomb '%s' hit at (%.1f, %.1f)", tile.name.c_str(), tile.position.x, tile.position.y);
    };

    bomb.onExplode = [](TileObject &tile, float radius)
    {
        TraceLog(LOG_INFO, "Bomb '%s' exploded! radius=%.1f at (%.1f, %.1f)", tile.name.c_str(), radius, tile.position.x, tile.position.y);
    };

    bomb.onDamagePlayer = [](TileObject &tile)
    {
        TraceLog(LOG_INFO, "Bomb '%s' damaged player", tile.name.c_str());
    };
}

/**
 * @brief Trigger ledakan bomb
 *
 * Urutan proses:
 * 1. Set state Inactive, tandai isExploding & isTriggered
 * 2. Hapus dari DynamicObstacles
 * 3. Damage player jika dalam radius
 * 4. Damage semua enemy aktif dalam radius
 * 5. Chain reaction: trigger Explode pada bomb lain dalam radius yang belum meledak
 *
 * isTriggered dipakai sebagai guard agar tidak terjadi infinite loop
 * pada layout bomb yang saling berdekatan.
 *
 * @param bomb BombData yang akan diledakkan
 * @param playerBounds Bounding box player
 * @param player Pointer ke player untuk apply damage
 */
void BombManager::Explode(BombData &bomb, Rectangle playerBounds, Player *player)
{
    bomb.tile.state = ObjectState::Inactive;
    bomb.isExploding = true;
    bomb.isTriggered = true;
    bomb.explosionTimer = BOMB_EXPLOSION_DURATION;

    consumedPositions.insert(EncodePos(bomb.tile.position));

    // hapus dari dynamic obstacles
    DynamicObstacles.erase(
        std::remove_if(DynamicObstacles.begin(), DynamicObstacles.end(), [&](const Rectangle &r)
                       { return r.x == bomb.tile.bounds.x && r.y == bomb.tile.bounds.y; }),
        DynamicObstacles.end());
    MarkSpawnFlowFieldsDirty(bomb.tile.position);
    RebuildObstacleCache();

    if (IsInExplosionRadius(bomb.tile.position, playerBounds))
        if (player)
            player->TakeDamage(BOMB_DAMAGE);
    // enemy nanti ditambah di

    for (auto entity : Entities::GetRegistry())
    {
        Enemy *enemy = dynamic_cast<Enemy *>(entity);
        if (!enemy)
            continue;
        if (!enemy->IsActive || enemy->Health <= 0)
            continue;
        if (IsInExplosionRadius(bomb.tile.position, enemy->GetHitbox()))
            enemy->TakeDamage(BOMB_DAMAGE, {0, 0});
    }

    // chain reaction
    for (auto &other : bombs)
    {
        if (!other.isAlive || other.isExploding || other.isTriggered)
            continue;
        if (IsInExplosionRadius(bomb.tile.position, other.tile.bounds))
            Explode(other, playerBounds, player);
    }
}

/**
 * @brief Cek apakah target rectangle berada dalam radius ledakan bomb
 *
 * Menggunakan nearest-point check dari center bomb ke rectangle target,
 * bukan center-to-center, agar akurat untuk target berukuran besar.
 *
 * @param bombPos Posisi center bomb
 * @param target Bounding box target
 * @return true jika jarak nearest point <= BOMB_EXPLOSION_RADIUS
 */
bool BombManager::IsInExplosionRadius(Vector2 bombPos, Rectangle target)
{
    // cek titik terdekat di rectangle target ke pusat bomb
    float nearestX = Clamp(bombPos.x, target.x, target.x + target.width);
    float nearestY = Clamp(bombPos.y, target.y, target.y + target.height);
    float dist = Vector2Distance(bombPos, {nearestX, nearestY});
    return dist <= BOMB_EXPLOSION_RADIUS;
}

/**
 * @brief Update state semua bomb tiap frame
 *
 * Countdown explosionTimer untuk bomb yang sedang meledak.
 * Bomb yang explosionTimer-nya habis di-set isAlive = false,
 * lalu dihapus dari vector di akhir update.
 *
 * @param deltaTime Waktu antar frame
 * @param playerBounds Bounding box player
 * @param player Pointer ke player
 */
void BombManager::Update(float deltaTime, Rectangle playerBounds, Player *player)
{
    for (auto &bomb : bombs)
    {
        if (!bomb.isAlive)
            continue;

        if (bomb.isExploding)
        {
            bomb.explosionTimer -= deltaTime;
            if (bomb.explosionTimer <= 0.0f)
                bomb.isAlive = false;
        }
    }

    // hapus bomb yang udah mati
    bombs.erase(
        std::remove_if(bombs.begin(), bombs.end(), [](const BombData &bomb)
                       { return !bomb.isAlive; }),
        bombs.end());
}

/**
 * @brief Trigger ledakan bomb yang terkena hitbox serangan player
 *
 * @param attackHitbox Hitbox serangan player
 * @param playerBounds Bounding box player
 * @param player Pointer ke player
 */
void BombManager::HitByAttack(Rectangle attackHitbox, Rectangle playerBounds, Player *player)
{
    for (auto &bomb : bombs)
    {
        if (!bomb.isAlive || bomb.isExploding)
            continue;
        if (!CheckCollisionAgainstRects(attackHitbox, {bomb.tile.bounds}))
            continue;
        Explode(bomb, playerBounds, player);
    }
}

/**
 * @brief Cari bomb terdekat dari titik hit
 *
 * @param hitPos Posisi hit
 * @param threshold Toleransi jarak ke tepi bounds
 * @return Pointer ke TileObject bomb terdekat, nullptr jika tidak ada
 */
TileObject *BombManager::FindBomb(Vector2 hitPos, float threshold)
{
    TileObject *closest = nullptr;
    float minDist = threshold;

    for (auto &bomb : bombs)
    {
        if (!bomb.isAlive)
            continue;
        if (IsHitInBounds(hitPos, bomb.tile.bounds, threshold))
        {
            float dist = DistToCenter(hitPos, bomb.tile.bounds);
            if (dist < minDist)
            {
                minDist = dist;
                closest = &bomb.tile;
            }
        }
    }
    return closest;
}

/**
 * @brief Render semua bomb ke layar
 *
 * Exploding = lingkaran orange transparan radius BOMB_EXPLOSION_RADIUS.
 * Idle = kotak RED. Placeholder, belum pakai sprite.
 */
int BombManager::Render(Rectangle viewRect)
{
    int rendered = 0;
    for (auto &bomb : bombs)
    {
        if (!bomb.isAlive)
            continue;
        if (!CheckCollisionRecs(bomb.tile.bounds, viewRect))
            continue;
        rendered++;

        if (bomb.isExploding)
            DrawCircleV(bomb.tile.position, BOMB_EXPLOSION_RADIUS, Fade(ORANGE, 0.4f));
        else
            DrawRectangleRec(bomb.tile.bounds, RED);
    }
    return rendered;
}

/**
 * @brief Bersihkan semua data bomb
 */
void BombManager::Clear()
{
    bombs.clear();
}
