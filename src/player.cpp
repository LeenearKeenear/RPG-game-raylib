#include "../include/player.h"
#include "../include/movement.h"
#include "../include/combat.h"
#include "../include/interaction.h"
#include "../include/inventory.h"
#include "../include/mapLogic.h"
#include "../include/debug.h"

Player PlayerInstance;

void Player::Init(GameState *state, const char *spawnObjectName)
{
    State = state;

    if (!isInitialized)
    {
        LoadTileTexture(TEXTURE_KNIGHT, "texture/knight.png");
        LoadTileTexture(TEXTURE_ITEMS,  "texture/test.png");

        MaxHealth = 100.0f;
        Health = MaxHealth;

        MaxMana = 100.0f;
        Mana = MaxMana;
        ManaRegenTimer = 0.0f;

        Hotbar[0] = {ITEM_WEAPON, "Iron Sword", 1, 10, 0, 6, 4};
        Hotbar[1] = {ITEM_WEAPON, "Wooden Bow", 1, 5, 0, 8, 4};
        Hotbar[2] = {ITEM_POTION, "Health Potion", 3, 0, 20, 7, 8};
        Hotbar[3] = {ITEM_POTION, "Mana Bread", 5, 0, 15, 10, 8};

        isInitialized = true;
        TraceLog(LOG_INFO, "Player: Global resources and stats initialized");
    }

    PlayAnimation(Anim, IDLE, DOWN, PlayerAnimationSet);
    CollisionRects.clear();
    CollisionPolygons.clear();

    if (tilesonMap == nullptr) {
        Position = {0.0f, 0.0f};
        Anim.position = Position;
        return;
    }

    Vector2 spawnPos;
    if (spawnObjectName && spawnObjectName[0] != '\0' && TiledHelperFunction.TryGetObjectPositionByName(spawnObjectName, spawnPos)) {
        Position = spawnPos;
    } else if (TiledHelperFunction.TryGetObjectPositionByName(SPAWN_OBJECT_NAME, spawnPos)) {
        Position = spawnPos;
    } else {
        Position = {((float)tilesonMap->width * 32) / 2.0f, ((float)tilesonMap->height * 32) / 2.0f};
    }

    Anim.position = Position;

    TiledHelper::CollisionResult collision;
    if (TiledHelperFunction.TryGetCollisionByLayerName(COLLISION_LAYER_NAME, collision)) {
        CollisionRects = collision.rects;
        CollisionPolygons = collision.polygons;
    }
}

void Player::Update()
{
    InputInstance.PollInput();
    InputInstance.UpdateState();

    if (InputInstance.IsRevive()) {
        Combat::HandleRevive(*this);
        return;
    }

    if (Anim.isDead) return;

    Combat::HandleCombat(*this);
    if (Anim.isDead) return;

    Inventory::HandleInventoryActions(*this);

    if (InputInstance.IsGoBack()) {
        pendingSwitchMap = false;
        pendingGoBack = true;
    }

    if (!Anim.isAttacking) {
        Movement::HandleMovement(*this);
    }

    Interaction::HandleInteractions(*this);

    UpdateAnimation(Anim, GetFrameTime());
    Movement::UpdateCamera(*this);
}

void Player::Render(void)
{
    DrawAnimation(Anim, TEXTURE_KNIGHT);
}