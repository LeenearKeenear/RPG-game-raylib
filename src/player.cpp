#include "../include/player.h"
#include "../include/movement.h"
#include "../include/combat.h"
#include "../include/interaction.h"
#include "../include/inventory.h"
#include "../include/mapLogic.h"
#include "../include/debug.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/propsbehavior.h"
#include <cmath>

Player PlayerInstance;

/**
 * Menginisialisasi karakter pemain.
 * Menangani pemuatan resource satu kali (tekstur) dan inisialisasi per map (spawn/tabrakan).
 */
void Player::Init(GameState *state, const char *spawnObjectName)
{
    State = state;

    // Memuat resource global pemain hanya satu kali
    if (!isInitialized)
    {
        LoadTileTexture(TEXTURE_KNIGHT, "texture/knight.png");
        LoadTileTexture(TEXTURE_ITEMS, "texture/test.png");
        LoadTileTexture(TEXTURE_ENEMIES, "texture/enemies.png");

        MaxHealth = 100.0f;
        Health = MaxHealth;

        MaxMana = 100.0f;
        Mana = MaxMana;
        ManaRegenTimer = 0.0f;

        // Inisialisasi perlengkapan hotbar default
        Hotbar[0] = {0, 1}; // Iron Sword
        Hotbar[1] = {1, 1}; // Iron Axe
        Hotbar[2] = {2, 3}; // Health Potion
        Hotbar[3] = {3, 5}; // Mana Bread

        for (int i = 0; i < 49; i++)
            Bag[i] = {-1, 0};

        isInitialized = true;
        TraceLog(LOG_INFO, "Player: Resource global dan statistik telah diinisialisasi");
    }

    PlayAnimation(Anim, IDLE, DOWN, PlayerAnimationSet);
    CollisionRects.clear();
    CollisionPolygons.clear();

    if (tilesonMap == nullptr)
    {
        Position = {0.0f, 0.0f};
        Anim.position = Position;
        return;
    }

    // Menentukan posisi spawn berdasarkan data map atau fallback ke titik tengah map
    Vector2 spawnPos;
    if (spawnObjectName && spawnObjectName[0] != '\0' && TiledHelperFunction.TryGetObjectPositionByName(spawnObjectName, spawnPos))
    {
        Position = spawnPos;
    }
    else if (TiledHelperFunction.TryGetObjectPositionByName(SPAWN_OBJECT_NAME, spawnPos))
    {
        Position = spawnPos;
    }
    else
    {
        Position = {((float)tilesonMap->width * 32) / 2.0f, ((float)tilesonMap->height * 32) / 2.0f};
    }

    Anim.position = Position;

    // Memuat geometri tabrakan statis dari map
    TiledHelper::CollisionResult collision;
    if (TiledHelperFunction.TryGetCollisionByLayerName(COLLISION_LAYER_NAME, collision))
    {
        CollisionRects = collision.rects;
        CollisionPolygons = collision.polygons;
    }
}

/**
 * Loop update utama untuk pemain.
 * Dipanggil oleh Entities::Update() setiap frame.
 *
 * Urutan:
 * 1. Input polling
 * 2. Lifecycle check (revive)
 * 3. Timer & Status effects
 * 4. Physics & Knockback
 * 5. Logic modules (Movement, Combat, Inventory, Interaction)
 * 6. Animation & Camera update
 * 7. Map transition handling
 */
void Player::Update()
{
    // 1. Memproses Input
    InputInstance.PollInput();
    InputInstance.UpdateState();

    // 2. Pemeriksaan Lifecycle
    if (InputInstance.IsRevive())
    {
        Combat::HandleRevive(*this);
        return;
    }

    if (Anim.isDead)
        return;

    // 3. Timer & Efek Status
    if (HitFlashTimer > 0)
        HitFlashTimer -= GetFrameTime();

    // 4. Fisika & Pergerakan (termasuk Knockback)
    if (Vector2Length(KnockbackVelocity) > 0.1f)
    {
        Vector2 nextPos = Vector2Add(Position, Vector2Scale(KnockbackVelocity, GetFrameTime() * 60.0f));
        if (Movement::CanMove(*this, nextPos))
        {
            Position = nextPos;
        }
        KnockbackVelocity = Vector2Scale(KnockbackVelocity, 0.85f); // Redaman gesekan
    }
    else
    {
        KnockbackVelocity = {0, 0};
    }

    // 5. Modul Logika
    if (InputInstance.IsGoBack())
    {
        pendingSwitchMap = false;
        pendingGoBack = true;
    }

    // Memblokir pergerakan selama animasi serangan
    if (!Anim.isAttacking)
    {
        Movement::HandleMovement(*this);
    }

    Combat::HandleCombat(*this);
    if (Anim.isDead)
        return;

    Inventory::HandleInventoryActions(*this);
    Interaction::HandleInteractions(*this);

    // 6. Update Animasi & Kamera
    Combat::UpdateSwingAttack(*this, GetFrameTime());
    Anim.position = Position;
    UpdateAnimation(Anim, GetFrameTime());
    Movement::UpdateCamera(*this);

    // 7. Handle map transitions
    if (pendingGoBack)
    {
        pendingGoBack = false;
        GoBack();
        return;
    }
    if (pendingSwitchMap)
    {
        pendingSwitchMap = false;
        SwitchMap(pendingMapPath.c_str(), pendingDoorName.c_str());
        return;
    }
}

/**
 * Render sprite player dengan animasi di posisi world saat ini.
 * Dipanggil dari Entities::Render() dengan Y-sorting.
 */
void Player::Render(void)
{
    // Bayangan (Drop shadow)
    DrawEllipse((int)Position.x + 16, (int)Position.y + 31, 10, 4, {0, 0, 0, 80});

    // Terapkan warna kilatan (merah saat terkena hit)
    Color tint = WHITE;
    if (HitFlashTimer > 0)
    {
        tint = RED;
    }

    DrawAnimation(Anim, TEXTURE_KNIGHT, tint);
    Combat::DrawSwingAttack(*this);
    DrawAimIndicator();
}

void Player::TakeDamage(float amount, Vector2 knockback)
{
    Entity::TakeDamage(amount, knockback);

    HitFlashTimer = 0.15f;
    KnockbackVelocity = Vector2Scale(knockback, 6.0f);

    if (Anim.isAttacking)
    {
        Swing.active = false;
        Anim.isAttacking = false;
        PlayAnimation(Anim, IDLE, Anim.direction, PlayerAnimationSet);
    }

    Vector2 center = {Position.x + 16, Position.y + 16};
    Combat::AddDamagePopup(center, amount);

    TraceLog(LOG_INFO, "PLAYER: Menerima %.1f damage. Sisa HP: %.1f", amount, Health);
}

/*==============================================================================
 * Private Helper Methods
 *==============================================================================*/

bool Player::CanMove(Vector2 newPosition)
{
    return Movement::CanMove(*this, newPosition);
}

Rectangle Player::GetPlayerHitboxAtPosition(Vector2 position)
{
    return {position.x + HitboxOffsetX, position.y + HitboxOffsetY, HitboxWidth, HitboxHeight};
}

/**
 * @brief Gambar indicator arah aim player (debug overlay)
 * Warna berubah berdasarkan apakah arah aim valid (dot product dengan facing direction)
 * Definisi: src/player.cpp (file ini)
 */
void Player::DrawAimIndicator(void)
{
    if (!isDebugMode)
        return;

    Vector2 playerCenter = GetCenter();
    Vector2 facingDir = {0, 0};
    switch (Anim.direction)
    {
    case UP:
        facingDir = {0, -1};
        break;
    case DOWN:
        facingDir = {0, 1};
        break;
    case LEFT:
        facingDir = {-1, 0};
        break;
    case RIGHT:
        facingDir = {1, 0};
        break;
    }

    Vector2 mouseWorld = GetScreenToWorld2D(GetVirtualMousePosition(State), camera);
    Vector2 aimDir = Vector2Normalize(Vector2Subtract(mouseWorld, playerCenter));
    Vector2 rayEnd = {
        playerCenter.x + aimDir.x * INTERACT_RANGE,
        playerCenter.y + aimDir.y * INTERACT_RANGE};

    float dot = Vector2DotProduct(facingDir, aimDir);
    Color indicatorColor = (dot >= RayCastAngle) ? GREEN : WHITE;

    DrawLineEx(playerCenter, rayEnd, 2.0f, indicatorColor);
    // Raycast hanya terhadap object layer (garis biru) untuk titik merah
    std::vector<MapObject> debugObstacles;
    if (tilesonMap)
    {
        for (auto &obj : tilesonMap->Objects)
        {
            if (obj.layerName == OBJECT_LAYER_NAME)
            {
                debugObstacles.push_back(obj);
            }
        }
    }
    LastHit = Ray.Cast(playerCenter, aimDir, INTERACT_RANGE, debugObstacles);

    // Titik merah hanya di interseksi dengan garis biru (object layer)
    if (LastHit.hit)
    {
        DrawCircleV(LastHit.point, 4.0f, RED);
    }
}

/*==============================================================================
 * Action Handlers
 *==============================================================================*/

/**
 * Resolve apa yang terjadi saat input aksi (misal: Left Click)
 * dilakukan berdasarkan context:
 * - Inventori terbuka → equip/unequip item
 * - Slot senjata (1/2) → attack
 * - Slot potion (3/4) → minum potion
 * Definisi: src/player.cpp (file ini)
 */
void Player::HandleAction(void)
{
    PlayerAction action = InputInstance.ResolveAction();

    switch (action)
    {
    case ACTION_ATTACK:
    {
        float manaCost = Inventory::GetAttackManaCost(*this);
        if (Mana >= manaCost)
        {
            UpdatePlayerAttack(Anim);
            Mana -= manaCost;
            ManaRegenTimer = ManaRegenDelay;
            TraceLog(LOG_INFO, "PLAYER: Attack! (slot %d) - Mana: %.1f", (int)InputInstance.GetActiveSlot(), Mana);
        }
        else
        {
            TraceLog(LOG_WARNING, "PLAYER: Attack failed! Out of energy.");
        }
        break;
    }

    case ACTION_DRINK_POTION:
    {
        int slotIdx = (int)InputInstance.GetActiveSlot() - 1;
        if (slotIdx >= 0 && slotIdx < 4)
        {
            InventoryItem &slot = Hotbar[slotIdx];
            if (slot.definitionId != -1 && slot.amount > 0)
            {
                const ItemDefinition &def = itemDefs.Get(slot.definitionId);
                if (def.category == ITEM_POTION)
                {
                    Inventory::UsePotion(*this, slotIdx);
                    TraceLog(LOG_INFO, "PLAYER: Drink potion! %s (slot %d)", def.name.c_str(), slotIdx + 1);
                }
            }
            else
            {
                TraceLog(LOG_INFO, "PLAYER: No potion in slot %d!", slotIdx + 1);
            }
        }
        break;
    }

    case ACTION_EQUIP_UNEQUIP:
        TraceLog(LOG_INFO, "PLAYER: Equip/Unequip from inventory!");
        break;

    case ACTION_NONE:
    default:
        break;
    }
}
