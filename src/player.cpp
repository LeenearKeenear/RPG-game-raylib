#include "../include/player.h"
#include "../include/movement.h"
#include "../include/combat.h"
#include "../include/interaction.h"
#include "../include/inventory.h"
#include "../include/mapLogic.h"
#include "../include/debug.h"
#include "../lib/raylib/include/raymath.h"

Player PlayerInstance;

void Player::Init(GameState *state, const char *spawnObjectName)
{
    State = state;

    if (!isInitialized)
    {
        LoadTileTexture(TEXTURE_KNIGHT, "texture/knight.png");
        LoadTileTexture(TEXTURE_ITEMS,  "texture/test.png");
        LoadTileTexture(TEXTURE_ENEMIES, "texture/enemies.png");

        MaxHealth = 100.0f;
        Health = MaxHealth;

        MaxMana = 100.0f;
        Mana = MaxMana;
        ManaRegenTimer = 0.0f;

        Hotbar[0] = {ITEM_WEAPON, "Iron Sword", 1, 10, 0, 6, 4};
        Hotbar[1] = {ITEM_WEAPON, "Iron Axe", 1, 5, 0, 7, 4};
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

    // Update Hit Flash Timer
    if (HitFlashTimer > 0) HitFlashTimer -= GetFrameTime();

    // Update Knockback
    if (Vector2Length(KnockbackVelocity) > 0.1f) {
        Vector2 nextPos = Vector2Add(Position, Vector2Scale(KnockbackVelocity, GetFrameTime() * 60.0f));
        // Gunakan Movement::CanMove untuk pengecekan collision player
        if (Movement::CanMove(*this, nextPos)) {
            Position = nextPos;
        }
        // Decay knockback
        KnockbackVelocity = Vector2Scale(KnockbackVelocity, 0.85f);
    } else {
        KnockbackVelocity = {0, 0};
    }

    if (InputInstance.IsGoBack()) {
        pendingSwitchMap = false;
        pendingGoBack = true;
    }

    if (!Anim.isAttacking) {
        Movement::HandleMovement(*this);
    }

    Combat::HandleCombat(*this);
    if (Anim.isDead) return;

    Inventory::HandleInventoryActions(*this);

    Interaction::HandleInteractions(*this);

    Combat::UpdateSwingAttack(*this, GetFrameTime());
    
    // Sinkronisasi posisi animasi dengan posisi entitas (WAJIB setiap frame agar tidak tertinggal saat knockback/attack)
    Anim.position = Position;

    UpdateAnimation(Anim, GetFrameTime());
    Movement::UpdateCamera(*this);
}

void Player::TakeDamage(float amount, Vector2 knockback) {
    Entity::TakeDamage(amount, knockback);
    
    // Trigger Hit Flash
    HitFlashTimer = 0.15f;
    
    // Trigger Knockback
    KnockbackVelocity = Vector2Scale(knockback, 6.0f); // Intensitas knockback diperbesar
    
    // Solusi bug: langsung akhiri attack player jika sedang menyerang
    if (Anim.isAttacking) {
        Swing.active = false;
        Anim.isAttacking = false;
        PlayAnimation(Anim, IDLE, Anim.direction, PlayerAnimationSet);
    }

    // Tambahkan Damage Popup
    Vector2 center = { Position.x + 16, Position.y + 16 };
    Combat::AddDamagePopup(center, amount);

    TraceLog(LOG_INFO, "PLAYER: Took %.1f damage. Remaining HP: %.1f", amount, Health);
}


void Player::Render(void)
{
    // Shadow sederhana (sama dengan enemy)
    DrawEllipse((int)Position.x + 16, (int)Position.y + 31, 10, 4, {0, 0, 0, 80});

    // Tentukan warna tint (Hit Flash Merah)
    Color tint = WHITE;
    if (HitFlashTimer > 0) {
        tint = RED;
    }

    DrawAnimation(Anim, TEXTURE_KNIGHT, tint);
    Combat::DrawSwingAttack(*this);
}
