#pragma once

#include "../lib/raylib/include/raylib.h"
#include "map.h"
#include "screen.h"
#include "tiles.h"
#include "animation.h"
#include "inventory.h"
#include "entity.h"
#include "input.h"
#include "mapLogic.h"

class Player;

namespace Movement {
    void HandleMovement(Player& player);
    void UpdateCamera(Player& player);
    bool CanMove(Player& player, Vector2 newPos);
}
namespace Combat {
    void HandleCombat(Player& player);
    void HandleRevive(Player& player);
}
namespace Interaction {
    void HandleInteractions(Player& player);
    void UpdateRaycast(Player& player);
    void CheckDoors(Player& player);
    void CheckProps(Player& player);
}
namespace Inventory {
    void HandleInventoryActions(Player& player);
    void UsePotion(Player& player, int slotIndex);
}

class Player : public Entity
{
public:
    void Init(GameState* state, const char *spawnObjectName = SPAWN_OBJECT_NAME);
    void Update() override;
    void Render(void) override;

    Vector2 Velocity = {0, 0};
    float Speed = 3.0f;
    float Mana = 100.0f;
    float MaxMana = 100.0f;
    float ManaRegenTimer = 0.0f;
    const float ManaRegenDelay = 2.0f;
    const float ManaRegenRate = 10.0f;
    const float AttackManaCost = 10.0f;

    Animation Anim;
    bool pendingSwitchMap = false;
    std::string pendingMapPath;
    std::string pendingDoorName;
    bool pendingGoBack = false;
    InventoryItem Hotbar[4];

    float HitboxWidth = 16.0f;
    float HitboxHeight = 12.0f;
    float HitboxOffsetX = 8.0f;
    float HitboxOffsetY = 14.0f;

    Rectangle GetHitbox() const override { return { Position.x + HitboxOffsetX, Position.y + HitboxOffsetY, HitboxWidth, HitboxHeight }; }

    std::vector<Rectangle> CollisionRects;
    std::vector<std::vector<Vector2>> CollisionPolygons;

    RayCast Ray;
    RayHitResult LastHit;
    const float INTERACT_RANGE = 32 * 2.0f;

    Vector2 GetPosition() { return Position; }
    float GetSpeed() { return Speed; }
    bool IsAlive() const override { return !Anim.isDead; }
    float GetHealth() { return Health; }
    float GetMaxHealth() { return MaxHealth; }
    float GetMana() { return Mana; }
    float GetMaxMana() { return MaxMana; }
    const char *GetName() { return Name; }
    RayHitResult GetLastHit() { return LastHit; }
    float GetHitboxWidth() { return HitboxWidth; }
    float GetHitboxHeight() { return HitboxHeight; }
    float GetHitboxOffsetX() { return HitboxOffsetX; }
    float GetHitboxOffsetY() { return HitboxOffsetY; }
    float GetINTERACT_RANGE() { return INTERACT_RANGE; }
    InventoryItem GetHotbarItem(int index) { return Hotbar[index]; }

    void SetHealth(float h) { Health = h; }
    void SetMana(float m) { Mana = m; }
    void SetHotbarItem(int index, InventoryItem item) { Hotbar[index] = item; }

    GameState* State = nullptr;

private:
    const char *Name = "Player Name";
    bool isInitialized = false;
};

extern Player PlayerInstance;
