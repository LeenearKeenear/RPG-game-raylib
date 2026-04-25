#include "../include/player.h"
#include "../include/movement.h"
#include "../include/combat.h"
#include "../include/interaction.h"
#include "../include/inventory.h"
#include "../include/mapLogic.h"
#include "../include/debug.h"
#include "../lib/raylib/include/raymath.h"

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
        LoadTileTexture(TEXTURE_ITEMS,  "texture/test.png");
        LoadTileTexture(TEXTURE_ENEMIES, "texture/enemies.png");

        MaxHealth = 100.0f;
        Health = MaxHealth;

        MaxMana = 100.0f;
        Mana = MaxMana;
        ManaRegenTimer = 0.0f;

        // Inisialisasi perlengkapan hotbar default
        Hotbar[0] = {ITEM_WEAPON, "Iron Sword", 1, 10, 0, 6, 4};
        Hotbar[1] = {ITEM_WEAPON, "Iron Axe", 1, 5, 0, 7, 4};
        Hotbar[2] = {ITEM_POTION, "Health Potion", 3, 0, 20, 7, 8};
        Hotbar[3] = {ITEM_POTION, "Mana Bread", 5, 0, 15, 10, 8};

        isInitialized = true;
        TraceLog(LOG_INFO, "Player: Resource global dan statistik telah diinisialisasi");
    }

    PlayAnimation(Anim, IDLE, DOWN, PlayerAnimationSet);
    CollisionRects.clear();
    CollisionPolygons.clear();

    if (tilesonMap == nullptr) {
        Position = {0.0f, 0.0f};
        Anim.position = Position;
        return;
    }

    // Menentukan posisi spawn berdasarkan data map atau fallback ke titik tengah map
    Vector2 spawnPos;
    if (spawnObjectName && spawnObjectName[0] != '\0' && TiledHelperFunction.TryGetObjectPositionByName(spawnObjectName, spawnPos)) {
        Position = spawnPos;
    } else if (TiledHelperFunction.TryGetObjectPositionByName(SPAWN_OBJECT_NAME, spawnPos)) {
        Position = spawnPos;
    } else {
        Position = {((float)tilesonMap->width * 32) / 2.0f, ((float)tilesonMap->height * 32) / 2.0f};
    }

    Anim.position = Position;

    // Memuat geometri tabrakan statis dari map
    TiledHelper::CollisionResult collision;
    if (TiledHelperFunction.TryGetCollisionByLayerName(COLLISION_LAYER_NAME, collision)) {
        CollisionRects = collision.rects;
        CollisionPolygons = collision.polygons;
    }
}

/**
 * Loop update utama untuk pemain.
 * Mengirimkan tugas ke modul khusus (Pergerakan, Pertarungan, Interaksi, dll.)
 */
void Player::Update()
{
    // 1. Memproses Input
    InputInstance.PollInput();
    InputInstance.UpdateState();

    // 2. Pemeriksaan Lifecycle
    if (InputInstance.IsRevive()) {
        Combat::HandleRevive(*this);
        return;
    }

    if (Anim.isDead) return;

    // 3. Timer & Efek Status
    if (HitFlashTimer > 0) HitFlashTimer -= GetFrameTime();

    // 4. Fisika & Pergerakan (termasuk Knockback)
    if (Vector2Length(KnockbackVelocity) > 0.1f) {
        Vector2 nextPos = Vector2Add(Position, Vector2Scale(KnockbackVelocity, GetFrameTime() * 60.0f));
        if (Movement::CanMove(*this, nextPos)) {
            Position = nextPos;
        }
        KnockbackVelocity = Vector2Scale(KnockbackVelocity, 0.85f); // Redaman gesekan
    } else {
        KnockbackVelocity = {0, 0};
    }

    // 5. Modul Logika
    if (InputInstance.IsGoBack()) {
        pendingSwitchMap = false;
        pendingGoBack = true;
    }

    // Memblokir pergerakan selama animasi serangan untuk memberikan efek "berat" pada pertarungan
    if (!Anim.isAttacking) {
        Movement::HandleMovement(*this);
    }

    Combat::HandleCombat(*this);
    if (Anim.isDead) return;

    Inventory::HandleInventoryActions(*this);
    Interaction::HandleInteractions(*this);

    // 6. Update Animasi & Kamera
    Combat::UpdateSwingAttack(*this, GetFrameTime());
    Anim.position = Position; // Sinkronisasi asal animasi dengan posisi fisika
    UpdateAnimation(Anim, GetFrameTime());
    Movement::UpdateCamera(*this);
}

/**
 * Menangani damage yang diterima dan menerapkan umpan balik visual/fisika.
 */
void Player::TakeDamage(float amount, Vector2 knockback) {
    Entity::TakeDamage(amount, knockback);
    
    HitFlashTimer = 0.15f;
    KnockbackVelocity = Vector2Scale(knockback, 6.0f);
    
    // Batalkan serangan saat ini jika terkena hit untuk mencegah spam "tukar damage"
    if (Anim.isAttacking) {
        Swing.active = false;
        Anim.isAttacking = false;
        PlayAnimation(Anim, IDLE, Anim.direction, PlayerAnimationSet);
    }

    // Munculkan angka damage di titik tengah pemain
    Vector2 center = { Position.x + 16, Position.y + 16 };
    Combat::AddDamagePopup(center, amount);

    TraceLog(LOG_INFO, "PLAYER: Menerima %.1f damage. Sisa HP: %.1f", amount, Health);
}

/**
 * Me-render sprite pemain dan efek visual terkait.
 */
void Player::Render(void)
{
    // Bayangan (Drop shadow)
    DrawEllipse((int)Position.x + 16, (int)Position.y + 31, 10, 4, {0, 0, 0, 80});

    // Terapkan warna kilatan (merah saat terkena hit)
    Color tint = WHITE;
    if (HitFlashTimer > 0) {
        tint = RED;
    }

    DrawAnimation(Anim, TEXTURE_KNIGHT, tint);
    Combat::DrawSwingAttack(*this); // Render efek serangan yang aktif
}
