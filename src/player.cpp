#include "../include/player.h"
#include "../include/player_input.h"
#include "../include/player_ui.h"
#include "../include/player_camera.h"
#include "../include/debug.h"
#include "../include/map.h"

// ================================================================
// Global Instance
// ================================================================
Player PlayerInstance;

// ================================================================
// Core logic
// ================================================================
void Player::Init(void)
{
    LoadTileTexture(TEXTURE_KNIGHT, "texture/Knight.png");

    MapObject *spawnObj = TilesonGetObjectByName(SPAWN_OBJECT_NAME);
    if (spawnObj != nullptr)
    {
        Position = {spawnObj->bounds.x, spawnObj->bounds.y};
        TraceLog(LOG_INFO, "Player: Spawn point found at (%.1f, %.1f)", Position.x, Position.y);
    }
    else
    {
        Position = {160.0f, 160.0f};
        TraceLog(LOG_WARNING, "Player: Spawn object '%s' not found, using default position", SPAWN_OBJECT_NAME);
    }

    std::vector<MapObject> collisionObjs = TilesonGetObjectsByType(COLLISION_LAYER_NAME);
    for (auto &obj : collisionObjs)
        CollisionRects.push_back(obj.bounds);

    TraceLog(LOG_INFO, "Player: Loaded %d collision rects", (int)CollisionRects.size());

    Hotbar[0] = {SLOT_WEAPON, "Sword",          false};
    Hotbar[1] = {SLOT_WEAPON, "Bow",            false};
    Hotbar[2] = {SLOT_POTION, "Health Potion",  false};
    Hotbar[3] = {SLOT_POTION, "Mana Potion",    false};

    SelectedHotbarSlot = 0;
    State = PLAYER_IDLE;
    bIsAlive = true;
    bInventoryOpen = false;
    bMapOpen = false;

    TraceLog(LOG_INFO, "Player: Dependencies and Handlers Initialized.");
}

HotbarSlot Player::GetHotbarSlot(int index)
{
    if (index >= 0 && index < 4)
        return Hotbar[index];

    return {SLOT_WEAPON, "Empty", true};
}

void Player::Update(void)
{
    if (!bIsAlive || bInventoryOpen || bMapOpen)
        return;
    if (State == PLAYER_ATTACKING || State == PLAYER_DRINKING_POTION)
        return;

    Vector2 NewPos = {
        Position.x + Velocity.x * Speed,
        Position.y + Velocity.y * Speed};

    if (CanMove(NewPos))
        Position = NewPos;
}

void Player::Render(void)
{
    if (bIsAlive)
    {
        RenderTilePNG(Position.x, Position.y, TILE_PLAYER_NEW, 0.0f, TEXTURE_KNIGHT);
    }
    else
    {
        RenderTilePNG(Position.x, Position.y, TILE_PLAYER_NEW, 0.0f, TEXTURE_KNIGHT);
    }
}

bool Player::CanMove(Vector2 NewPos)
{
    Rectangle playerBox = {NewPos.x, NewPos.y, (float)TileSize, (float)TileSize};

    for (auto &rect : CollisionRects)
    {
        if (CheckCollisionRecs(playerBox, rect))
            return false;
    }

    return true;
}

// ================================================================
// Delegation to Handlers
// ================================================================

void Player::PlayerCamera(void)
{
    PlayerCameraManager::PlayerCamera(this);
}

TileRange Player::GetVisibleTileRange(void)
{
    return PlayerCameraManager::GetVisibleTileRange(this);
}

void Player::Tick(void)
{
    // Delegate to Input Component
    PlayerInput::HandleInput(this);
    
    // Core Update
    Update();
    
    // Delegate to Camera Component
    PlayerCamera();
}

void Player::RenderHUD(void)
{
    // Delegate to UI Component
    PlayerUI::RenderHUD(this);
}