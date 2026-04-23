#pragma once

#include "../lib/raylib/include/raylib.h"
#include "map.h"
#include "screen.h"
#include "tiles.h"
#include "animation.h"
#include "input.h"
#include "inventory.h"

class Player
{
public:
    void Init(GameState* state, const char *spawnObjectName = SPAWN_OBJECT_NAME);

    void Update();

    void Render(void);

    void Tick();

    void PlayerCamera(void);

    Vector2 GetPosition() { return Position; }

    float GetSpeed() { return Speed; }

    bool IsAlive() { return !Anim.isDead; }

    float GetHitboxWidth() { return HitboxWidth; }
    float GetHitboxHeight() { return HitboxHeight; }
    float GetHitboxOffsetX() { return HitboxOffsetX; }
    float GetHitboxOffsetY() { return HitboxOffsetY; }

    float GetHealth() { return Health; }
    float GetMaxHealth() { return MaxHealth; }
    void SetHealth(float h) { Health = h; }

    float GetMana() { return Mana; }
    float GetMaxMana() { return MaxMana; }
    void SetMana(float m) { Mana = m; }

    RayHitResult GetLastHit() { return LastHit; }
    float GetINTERACT_RANGE() { return INTERACT_RANGE; }

    const char *GetName() { return Name; }

    Animation Anim;

    bool pendingSwitchMap = false;
    std::string pendingMapPath;
    std::string pendingDoorName;
    bool pendingGoBack = false;

    InventoryItem GetHotbarItem(int index) { return Hotbar[index]; }
    void SetHotbarItem(int index, InventoryItem item) { Hotbar[index] = item; }
    void UsePotion(int slotIndex);

private:
    Rectangle GetPlayerHitboxAtPosition(Vector2 position);

    bool CanMove(Vector2 NewPos);

    void RayCasting();

    void CheckDoorInteraction(void);

    void CheckPropInteraction(void);

    void HandleSpaceAction(void);

    void HandleRevive(void);

    GameState* State = nullptr;

    Vector2 Position;
    Vector2 Velocity;
    int TileSize = 32;
    float Speed = 3.0f;
    Texture2D CharTexture;
    const char *Name = "Player Name";

    float HitboxWidth = 16.0f;
    float HitboxHeight = 12.0f;
    float HitboxOffsetX = 8.0f;
    float HitboxOffsetY = 14.0f;

    std::vector<Rectangle> CollisionRects;
    std::vector<std::vector<Vector2>> CollisionPolygons;

    std::vector<Vector2> WorldBoundaryPolygon;

    float Health = 100.0f;
    float MaxHealth = 100.0f;

    float Mana = 100.0f;
    float MaxMana = 100.0f;
    float ManaRegenTimer = 0.0f;
    const float ManaRegenDelay = 2.0f;
    const float ManaRegenRate = 10.0f;
    const float AttackManaCost = 10.0f;

    void HandleAction(void);

    RayCast Ray;
    RayHitResult LastHit;

    const float INTERACT_RANGE = TILE_SIZE * 2.0f;
    InventoryItem Hotbar[4];

    bool isInitialized = false;
};

extern Player PlayerInstance;
