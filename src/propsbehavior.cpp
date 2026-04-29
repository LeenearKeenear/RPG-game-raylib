#include "../include/propsbehavior.h"
#include "../include/item.h"
#include "../include/enemy.h"
#include "../include/entities.h"

Vector2 SnapToTileGrid(Vector2 rawPos)
{
    return {
        std::floor(rawPos.x / TILE_SIZE) * TILE_SIZE,
        std::floor(rawPos.y / TILE_SIZE) * TILE_SIZE};
}

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

float DistToCenter(Vector2 hitPos, Rectangle bounds)
{
    Vector2 center = {bounds.x + bounds.width / 2, bounds.y + bounds.height / 2};
    return Vector2Distance(hitPos, center);
}

void SpawnObject()
{
    auto chestObjs = TiledHelper::GetObjectsByType(CHEST_TYPE_OBJECT_NAME);
    chestManager.SpawnChests(chestObjs);

    auto spikeObjs = TiledHelper::GetObjectsByType(SPIKE_TYPE_OBJECT_NAME);
    spikeManager.SpawnSpikes(spikeObjs);

    auto bombObjs = TiledHelper::GetObjectsByType(BOMB_TYPE_OBJECT_NAME);
    bombManager.SpawnBombs(bombObjs);
}

// chest
ChestManager chestManager;

void ChestManager::SpawnChests(const std::vector<MapObject *> &chestObjects)
{
    chests.clear();
    for (auto *obj : chestObjects)
    {
        TileObject c;
        c.name = obj->name;
        c.bounds = obj->bounds;
        c.state = ObjectState::Closed;
        c.position = SnapToTileGrid({obj->bounds.x, obj->bounds.y});

        TraceLog(LOG_INFO, "Chest '%s': raw=(%.1f,%.1f) snap=(%.1f,%.1f) size=(%.1f,%.1f)",
                 obj->name.c_str(),
                 obj->bounds.x, obj->bounds.y,
                 c.position.x, c.position.y,
                 obj->bounds.width, obj->bounds.height);

        chests.push_back(c);
    }
}

/**
 * @brief Cari chest terdekat dari titik hit
 * Menggunakan expanded bounds agar titik di tepi tetap terdeteksi.
 * Definisi: src/propsbehavior.cpp (file ini)
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

void ChestManager::Interact(Vector2 hitPos)
{
    TraceLog(LOG_INFO, "Looking for chest at (%.1f, %.1f), total chests: %d", hitPos.x, hitPos.y, (int)chests.size());
    TileObject *chest = FindChest(hitPos);
    if (!chest || chest->state == ObjectState::Open)
        return;
    chest->state = ObjectState::Open;
    TriggerLoot(*chest);
}

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

void ChestManager::Render()
{
    // placeholder: switch gambar based on state
    for (auto &c : chests)
    {
        if (c.state == ObjectState::Closed)
            DrawRectangleRec(c.bounds, BROWN); // placeholder harus diganti pake skin
        else
            DrawRectangleRec(c.bounds, WHITE); // placeholder harus diganti pake skin
    }
}

void ChestManager::Clear()
{
    chests.clear();
}

// trap spike
SpikeManager spikeManager;

unsigned int SpikeManager::SeedFromName(const std::string &name)
{
    unsigned int seed = 0;
    for (char c : name)
        seed = seed * 31 + static_cast<unsigned int>(c);
    return seed;
}

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

        TraceLog(LOG_INFO, "Spike '%s': raw=(%.1f,%.1f) snap=(%.1f,%.1f) size=(%.1f,%.1f)",
                 obj->name.c_str(),
                 obj->bounds.x, obj->bounds.y,
                 data.tile.position.x, data.tile.position.y,
                 obj->bounds.width, obj->bounds.height);

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

        TraceLog(LOG_INFO, "Spike '%s': active=%.1fs inactive=%.1fs",
                 obj->name.c_str(),
                 data.activeDuration,
                 data.inactiveDuration);

        SetupCallbacks(data);
        spikes.push_back(data);
    }
}

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

void SpikeManager::Render()
{
    for (auto &spike : spikes)
    {
        if (spike.tile.state == ObjectState::Active)
            DrawRectangleRec(spike.tile.bounds, RED); // placeholder harus diganti pake skin
        else
            DrawRectangleRec(spike.tile.bounds, GRAY); // placeholder harus diganti pake skin
    }
}

void SpikeManager::Clear()
{
    spikes.clear();
}

// bomb
BombManager bombManager;

void BombManager::SpawnBombs(const std::vector<MapObject *> &bombObjects)
{
    bombs.clear();
    spawnPoints.clear();
    for (auto *obj : bombObjects)
    {
        BombData data;
        data.tile.name = obj->name;
        data.tile.bounds = obj->bounds;
        data.tile.state = ObjectState::Active;
        data.tile.position = SnapToTileGrid({obj->bounds.x, obj->bounds.y});

        TraceLog(LOG_INFO, "Bomb '%s': raw=(%.1f,%.1f) snap=(%.1f,%.1f) size=(%.1f,%.1f)",
                 obj->name.c_str(),
                 obj->bounds.x, obj->bounds.y,
                 data.tile.position.x, data.tile.position.y,
                 obj->bounds.width, obj->bounds.height);

        data.isAlive = true;
        data.isExploding = false;
        data.explosionTimer = 0.0f;

        SetupCallbacks(data);
        bombs.push_back(data);
        spawnPoints.push_back(data.tile.position);
        DynamicObstacles.push_back(data.tile.bounds);
    }
}

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

void BombManager::Explode(BombData &bomb, Rectangle playerBounds, Player *player)
{
    bomb.tile.state = ObjectState::Inactive;
    bomb.isExploding = true;
    bomb.explosionTimer = BOMB_EXPLOSION_DURATION;

    // hapus dari dynamic obstacles
    DynamicObstacles.erase(
        std::remove_if(DynamicObstacles.begin(), DynamicObstacles.end(), [&](const Rectangle &r)
                       { return r.x == bomb.tile.bounds.x && r.y == bomb.tile.bounds.y; }),
        DynamicObstacles.end());

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
}

bool BombManager::IsInExplosionRadius(Vector2 bombPos, Rectangle target)
{
    // cek titik terdekat di rectangle target ke pusat bomb
    float nearestX = Clamp(bombPos.x, target.x, target.x + target.width);
    float nearestY = Clamp(bombPos.y, target.y, target.y + target.height);
    float dist = Vector2Distance(bombPos, {nearestX, nearestY});
    return dist <= BOMB_EXPLOSION_RADIUS;
}

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

void BombManager::Render()
{
    for (auto &bomb : bombs)
    {
        if (!bomb.isAlive)
            continue;

        if (bomb.isExploding)
            DrawCircleV(bomb.tile.position, BOMB_EXPLOSION_RADIUS, Fade(ORANGE, 0.4f));
        else
            DrawRectangleRec(bomb.tile.bounds, RED); // placeholder, ganti sprite
    }
}

void BombManager::Clear()
{
    bombs.clear();
}

void BombManager::SpawnAll()
{
    // // debug: spawn ulang semua bomb dari spawnPoints
    // bombs.clear();
    // for (size_t i = 0; i < spawnPoints.size(); i++)
    // {
    //     std::string name = "bomb_debug_" + std::to_string(i);
    //     Spawn(spawnPoints[i], name);
    // }
}