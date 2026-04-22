#include "../include/propsbehavior.h"

ChestManager chestManager;

Vector2 SnapToTileGrid(Vector2 rawPos)
{
    return {
        std::floor(rawPos.x / TILE_SIZE) * TILE_SIZE,
        std::floor(rawPos.y / TILE_SIZE) * TILE_SIZE};
}


void SpawnObject(){
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

TileObject *ChestManager::FindChest(const std::string &name)
{
    for (auto &c : chests)
    {
        TraceLog(LOG_INFO, "Checking chest: '%s'", c.name.c_str());
        if (c.name == name)
            return &c;
    }

    return nullptr;
}

void ChestManager::Interact(const std::string &chestName)
{
    TraceLog(LOG_INFO, "Looking for chest: '%s', total chests: %d", chestName.c_str(), (int)chests.size());
    TileObject *chest = FindChest(chestName);

    if (!chest || chest->state == ObjectState::Open)
        return;

    chest->state = ObjectState::Open;
    TriggerLoot(chestName);
}

void ChestManager::TriggerLoot(const std::string &chestName)
{
    // TODO: ganti keyword rarity sesuai kesepakatan nama dengan temen
    if (chestName.find("common") != std::string::npos)
    {
        // TODO: spawn common loot
        TraceLog(LOG_INFO, "Spawning common loot for: %s", chestName.c_str());
    }
    else if (chestName.find("rare") != std::string::npos)
    {
        // TODO: spawn rare loot
        TraceLog(LOG_INFO, "Spawning rare loot for: %s", chestName.c_str());
    }
    else if (chestName.find("epic") != std::string::npos)
    {
        // TODO: spawn epic loot
        TraceLog(LOG_INFO, "Spawning epic loot for: %s", chestName.c_str());
    }
    else
    {
        // TODO: fallback jika nama chest tidak mengandung rarity keyword
        TraceLog(LOG_WARNING, "Unknown rarity for chest: %s", chestName.c_str());
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
