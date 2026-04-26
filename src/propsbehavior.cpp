#include "../include/propsbehavior.h"

extern void SpawnItemAtLocation(Vector2 pos);
ChestManager chestManager;

Vector2 SnapToTileGrid(Vector2 rawPos)
{
    return {
        std::floor(rawPos.x / TILE_SIZE) * TILE_SIZE,
        std::floor(rawPos.y / TILE_SIZE) * TILE_SIZE};
}

void SpawnObject()
{
    auto chestObjs = TiledHelper::GetObjectsByType(CHEST_TYPE_OBJECT_NAME);
    chestManager.SpawnChests(chestObjs);
}

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

TileObject *ChestManager::FindChest(Vector2 hitPos, float threshold)
{
    TileObject *closest = nullptr;
    float minDist = threshold;

    for (auto &chest : chests)
    {
        float dist = Vector2Distance(hitPos, chest.position);
        if (dist < minDist)
        {
            minDist = dist;
            closest = &chest;
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
            chest.position.x + (float)GetRandomValue(-20, 20),
            chest.position.y + (float)GetRandomValue(-20, 20)
        };
        
        SpawnItemAtLocation(spawnPos);
    }
}

void ChestManager::Render()
{
    // placeholder: switch gambar based on state
    for (auto &c : chests)
    {
        if (c.state == ObjectState::Closed)
            DrawRectangleRec(c.bounds, BROWN);
        else
            DrawRectangleRec(c.bounds, YELLOW);
    }
}

void ChestManager::Clear()
{
    chests.clear();
}
