#include "../include/mapLogic.h"
#include "../include/player.h"
#include "../include/debug.h"
#include "../include/map.h"
#include "../include/animation.h"
#include "../include/input.h"
#include <cmath>

// ================================================================
// Global
// ================================================================

// global instance player — diakses file lain via extern
Player PlayerInstance;

// ================================================================
// Init()
// Inisialisasi player: load texture, ambil spawn point dari
// object layer Tiled (nama object: SPAWN_OBJECT_NAME "spawn"),
// dan load semua collision rectangles dari object layer
// (layer: COLLISION_LAYER_NAME "collision")
//
// Cara kerja:
// 1. Load texture character dari file png
// 2. Cari object "spawn" di tilesonMap->Objects → set Position
// 3. Ambil semua object dari layer "collision" → simpen di CollisionRects / CollisionPolygons
// 4. Ambil custom world boundary polygon dari layer "map_bound"
// ================================================================
void Player::Init(void)
{
    // TODO: path texture disesuaiin sama asset yang ada
    LoadTileTexture(TEXTURE_KNIGHT, "texture/Knight.png");
    LoadTileTexture(TEXTURE_ITEMS,  "texture/test.png");

    // inisialisasi state animasi player
    Anim.position = {0.0f, 0.0f};
    Anim.state = IDLE;
    Anim.direction = DOWN;
    Anim.frame = 0;
    Anim.frameTime = 0.0f;
    Anim.frameSpeed = 0.5f;
    Anim.walkFrameIndex = 0;
    Anim.isAttacking = false;
    Anim.isDead = false;

    // init health
    MaxHealth = 100.0f;
    Health = MaxHealth;

    // init mana
    MaxMana = 100.0f;
    Mana = MaxMana;
    ManaRegenTimer = 0.0f;

    // reset collision cache biar aman kalau nanti map di-reload
    CollisionRects.clear();
    CollisionPolygons.clear();

    // safety check kalau map belum keload
    if (tilesonMap == nullptr)
    {
        Position = {0.0f, 0.0f};
        Anim.position = Position;
        TraceLog(LOG_ERROR, "Player: tilesonMap is null during Init()");
        return;
    }

    // ================================================================
    // ambil spawn point dari object layer Tiled
    // pakai TiledHelperFunction.TryGetObjectPositionByName
    // ================================================================
    Vector2 spawnPos;
    if (TiledHelperFunction.TryGetObjectPositionByName(SPAWN_OBJECT_NAME, spawnPos))
    {
        Position = spawnPos;
        TraceLog(LOG_INFO, "Player: Spawn point found at (%.1f, %.1f)", Position.x, Position.y);
    }
    else
    {
        // fallback kalau object spawn belum ada di Tiled
        // posisi fallback dipusatkan ke tengah map dalam pixel
        Position = {
            (((float)tilesonMap->width * TILE_SIZE) / 2.0f) + TILE_SIZE * 3,
            ((float)tilesonMap->height * TILE_SIZE) / 2.0f};
        TraceLog(LOG_WARNING, "Player: Spawn object '%s' not found, using default position", SPAWN_OBJECT_NAME);
    }

    // sync posisi animasi dengan posisi player
    Anim.position = Position;

    // ambil semua collision object dari object layer Tiled
    // rectangle disimpan ke CollisionRects, polygon disimpan ke CollisionPolygons
    TiledHelper::CollisionResult collision;
    if (TiledHelperFunction.TryGetCollisionByLayerName(COLLISION_LAYER_NAME, collision))
    {
        CollisionRects = collision.rects;
        CollisionPolygons = collision.polygons;
    }

    TraceLog(LOG_INFO, "Player: Loaded %d collision rects", (int)CollisionRects.size());
    TraceLog(LOG_INFO, "Player: Loaded %d collision polygons", (int)CollisionPolygons.size());
    TraceLog(LOG_INFO, "Player: Custom world boundary %s", WorldBoundaryPolygon.empty() ? "not found" : "loaded");

    // Initialize default hotbar items
    Hotbar[0] = {ITEM_WEAPON, "Iron Sword", 1, 10, 0, 6, 4};
    Hotbar[1] = {ITEM_WEAPON, "Wooden Bow", 1, 5, 0, 8, 4};
    Hotbar[2] = {ITEM_POTION, "Health Potion", 3, 0, 20, 7, 8};
    Hotbar[3] = {ITEM_POTION, "Bread", 5, 0, 10, 10, 8}; // Diubah ke Makanan (Bread)
}

// ================================================================
// Update()
// Handle input keyboard dan movement player per frame.
// Normalisasi velocity biar diagonal gak lebih cepet dari cardinal.
// Cek collision sebelum apply posisi baru.
// ================================================================
void Player::Update(void)
{
    // poll input di awal frame
    InputInstance.PollInput();
    InputInstance.UpdateState();

    // --- Revive (R) — bisa dipanggil kapan saja, bahkan saat dead ---
    if (InputInstance.IsRevive())
    {
        HandleRevive();
        return;
    }

    // kalau player dead, skip semua input selain revive
    if (Anim.isDead) return;

    // cek kalau HP habis
    if (Health <= 0)
    {
        Health = 0;
        UpdatePlayerDeath(Anim);
        return;
    }


    // --- Health Test (DEBUG) ---
    if (InputInstance.IsTestLoseHP())
    {
        Health -= 10.0f;
        if (Health < 0) Health = 0;
        TraceLog(LOG_INFO, "PLAYER: Test Health Decrease (%.1f)", Health);
    }

    // --- Left Click — context action (attack / potion / equip) ---
    if (InputInstance.IsLeftClickPressed() && !Anim.isAttacking)
    {
        HandleSpaceAction();
        // kalau jadi attack, skip movement frame ini
        if (Anim.isAttacking) return;
    }

    // kalau lagi attack, skip movement
    if (Anim.isAttacking) return;

    // --- Interact (E) ---
    if (InputInstance.IsInteract())
    {
        // TODO: implementasi interact dengan object/NPC di depan player
        TraceLog(LOG_INFO, "PLAYER: Interact triggered");
    }

    // --- Movement (Arrow/WASD) ---
    Velocity = {0, 0};
    bool moving = false;

    if (InputInstance.IsMoveUp())
    {
        Velocity.y -= 1;
        Anim.direction = UP;
        moving = true;
    }
    if (InputInstance.IsMoveDown())
    {
        Velocity.y += 1;
        Anim.direction = DOWN;
        moving = true;
    }
    if (InputInstance.IsMoveLeft())
    {
        Velocity.x -= 1;
        Anim.direction = LEFT;
        moving = true;
    }
    if (InputInstance.IsMoveRight())
    {
        Velocity.x += 1;
        Anim.direction = RIGHT;
        moving = true;
    }

    // set animation state berdasarkan movement
    if (moving)
        Anim.state = WALK;
    else
        Anim.state = IDLE;

    // normalisasi biar diagonal gak lebih cepet dari cardinal
    float Length = sqrtf(Velocity.x * Velocity.x + Velocity.y * Velocity.y);
    if (Length != 0)
    {
        Velocity.x /= Length;
        Velocity.y /= Length;
    }

    Vector2 NewPos = {
        Position.x + Velocity.x * Speed,
        Position.y + Velocity.y * Speed};

    if (CanMove(NewPos))
        Position = NewPos;

    // sync posisi animasi dengan posisi player
    Anim.position = Position;

    // --- Mana Regeneration ---
    // Regenerasi mana aktif jika timer sudah 0 (3 detik setelah attack terakhir)
    if (ManaRegenTimer > 0)
    {
        ManaRegenTimer -= GetFrameTime();
    }
    else
    {
        if (Mana < MaxMana)
        {
            Mana += ManaRegenRate * GetFrameTime();
            if (Mana > MaxMana) Mana = MaxMana;
        }
    }
}

// ================================================================
// Render()
// Render sprite player dengan animasi di posisi world saat ini.
// Menggunakan DrawPlayer() dari animation system.
// Dipanggil dari RenderEntities() setelah RenderMap().
// ================================================================
void Player::Render(void)
{
    DrawPlayer(Anim);
}

// ================================================================
// HandleSpaceAction()
// Resolve apa yang terjadi saat SPACE ditekan berdasarkan context:
// - Inventori terbuka → equip/unequip item
// - Slot senjata (1/2) → attack
// - Slot potion (3/4) → minum potion
// ================================================================
void Player::HandleSpaceAction(void)
{
    SpaceAction action = InputInstance.ResolveSpaceAction();

    switch (action)
    {
    case ACTION_ATTACK:
        if (Mana >= AttackManaCost)
        {
            UpdatePlayerAttack(Anim);
            Mana -= AttackManaCost;
            ManaRegenTimer = ManaRegenDelay; // Reset timer ke 3 detik
            TraceLog(LOG_INFO, "PLAYER: Attack! (slot %d) - Mana: %.1f", (int)InputInstance.GetActiveSlot(), Mana);
        }
        else
        {
            TraceLog(LOG_WARNING, "PLAYER: Attack failed! Out of energy.");
        }
        break;

    case ACTION_DRINK_POTION:
    {
        int slotIdx = (int)InputInstance.GetActiveSlot() - 1;
        if (slotIdx >= 0 && slotIdx < 4) {
            if (Hotbar[slotIdx].type == ITEM_POTION && Hotbar[slotIdx].amount > 0) {
                UsePotion(slotIdx);
                TraceLog(LOG_INFO, "PLAYER: Drink potion! %s (slot %d)", Hotbar[slotIdx].name.c_str(), (int)InputInstance.GetActiveSlot());
            } else {
                TraceLog(LOG_INFO, "PLAYER: No potion in slot %d!", (int)InputInstance.GetActiveSlot());
            }
        }
        break;
    }

    case ACTION_EQUIP_UNEQUIP:
        // TODO: implementasi equip/unequip dari inventori
        TraceLog(LOG_INFO, "PLAYER: Equip/Unequip from inventory!");
        break;

    case ACTION_NONE:
    default:
        break;
    }
}

// ================================================================
// UsePotion()
// Terapkan efek potion dan kurangi jumlahnya.
// ================================================================
void Player::UsePotion(int slotIndex)
{
    if (Hotbar[slotIndex].type != ITEM_POTION || Hotbar[slotIndex].amount <= 0)
        return;

    // Apply heal
    Health += Hotbar[slotIndex].healValue;
    if (Health > MaxHealth) Health = MaxHealth;

    // TODO: if Mana potion, apply mana heal

    // Consume item
    Hotbar[slotIndex].amount--;
    if (Hotbar[slotIndex].amount <= 0) {
        Hotbar[slotIndex] = {ITEM_NONE, "", 0, 0, 0, 0, 0};
    }
}

// ================================================================
// HandleRevive()
// Reset player dari state DEAD ke IDLE.
// Dipanggil saat R ditekan (debug/testing).
// ================================================================
void Player::HandleRevive(void)
{
    if (Anim.isDead)
    {
        Anim.isDead = false;
        Anim.isAttacking = false;
        Anim.state = IDLE;
        Anim.frame = 0;
        Anim.frameTime = 0.0f;
        Health = MaxHealth; // Reset health
        Mana = MaxMana;     // Reset mana
        TraceLog(LOG_INFO, "PLAYER: Revived!");
    }
}

// ================================================================
// CanMove()
// Cek apakah posisi baru player (NewPos) nabrak salah satu
// collision shape dari object layer Tiled, atau keluar dari
// world boundary.
//
// Collision box player diasumsiin 1 tile (TileSize x TileSize).
// Return false kalau nabrak / keluar bound, true kalau aman.
// ================================================================
bool Player::CanMove(Vector2 newPosition)
{
    // --- Build hitbox at new position ---
    Rectangle hitbox = BuildHitbox(newPosition, HitboxOffsetX, HitboxOffsetY,
                                   HitboxWidth, HitboxHeight);

    // --- World bounds check ---
    if (!IsWithinWorldBounds(hitbox, tilesonMap->width * TILE_SIZE, tilesonMap->height * TILE_SIZE))
        return false;

    // --- Check rect collisions ---
    if (CheckCollisionAgainstRects(hitbox, CollisionRects))
        return false;

    // --- Check polygon collisions ---
    if (CheckCollisionAgainstPolygons(hitbox, CollisionPolygons))
        return false;

    return true;
}

// ================================================================
// PlayerCamera()
// Handle camera follow player dengan clamp ke world bounds.
//
// Cara kerja:
// 1. Hitung zoom — auto kalau map <= MinMapTileZoom, FixedZoom kalau lebih
// 2. Camera target selalu nge-follow tengah player
// 3. Camera di-clamp biar gak keluar dari world bounds
//
// Catatan: FixedZoom bisa diubah manual sesuai kebutuhan map
// ================================================================
void Player::PlayerCamera(void)
{
    float MapW = (float)tilesonMap->width * TILE_SIZE;
    float MapH = (float)tilesonMap->height * TILE_SIZE;

    // zoom otomatis kalau map <= MinMapTileZoom biar map ngisi viewport
    // kalau map lebih gede, pake FixedZoom — ubah nilai ini buat adjust
    const int MinMapTileZoom = 15;
    float AutoZoom = (float)GameScreenWidth / (MinMapTileZoom * TILE_SIZE);
    float FixedZoom = 2.0f;
    const float CameraZoom = (tilesonMap->width <= MinMapTileZoom || tilesonMap->height <= MinMapTileZoom)
                                 ? AutoZoom
                                 : FixedZoom;

    // kalau debug mode off, pakai zoom yang udah dihitung
    if (!isDebugMode)
        camera.zoom = CameraZoom;

    // camera target = tengah player
    camera.target.x = PlayerInstance.GetPosition().x + (TILE_SIZE / 2.0f);
    camera.target.y = PlayerInstance.GetPosition().y + (TILE_SIZE / 2.0f);

    // hitung half viewport dalam world space
    float halfW = (GameScreenWidth / 2.0f) / camera.zoom;
    float halfH = (GameScreenHeight / 2.0f) / camera.zoom;

    // clamp camera target ke world bounds
    // camera berhenti ngikutin player kalau udah di tepi map
    if (MapW <= halfW * 2.0f)
        camera.target.x = MapW / 2.0f;
    else
    {
        if (camera.target.x < halfW)
            camera.target.x = halfW;
        if (camera.target.x > MapW - halfW)
            camera.target.x = MapW - halfW;
    }

    if (MapH <= halfH * 2.0f)
        camera.target.y = MapH / 2.0f;
    else
    {
        if (camera.target.y < halfH)
            camera.target.y = halfH;
        if (camera.target.y > MapH - halfH)
            camera.target.y = MapH - halfH;
    }
}

// ================================================================
// Tick()
// Wrapper logic player per frame — dipanggil dari UpdateLogicAll().
// Urutan: update input/movement → update animasi → update camera
// ================================================================
void Player::Tick(void)
{
    Update();
    UpdateAnimation(Anim, GetFrameTime());
    PlayerCamera();
}

// ================================================================
// GetVisibleTileRange()
// Inti logic frustum culling — hitung range tile yang visible
// di layar berdasarkan camera viewport saat ini.
//
// Cara kerja:
// 1. Konversi pojok kiri-atas layar ke world space → worldMin
// 2. Konversi pojok kanan-bawah layar ke world space → worldMax
// 3. Bagi koordinat world dengan TILE_SIZE → dapat index tile
// 4. Tambah margin 1 tile di tiap sisi biar gak ada pop-in di tepi
// 5. Clamp ke batas map yang valid (0 .. width/height)
//
// Dipanggil oleh RenderMapCulled() (frustum.cpp) setiap frame.
// ================================================================
TileRange Player::GetVisibleTileRange(void)
{
    // pojok kiri-atas dan kanan-bawah layar dalam world space
    Vector2 worldMin = GetScreenToWorld2D({0.0f, 0.0f}, camera);
    Vector2 worldMax = GetScreenToWorld2D({(float)GameScreenWidth, (float)GameScreenHeight}, camera);

    TileRange range;

    // konversi ke tile index + margin 1 tile biar gak ada pop-in di tepi
    range.minX = (int)floorf(worldMin.x / TILE_SIZE) - 1;
    range.minY = (int)floorf(worldMin.y / TILE_SIZE) - 1;
    range.maxX = (int)ceilf(worldMax.x / TILE_SIZE) + 1;
    range.maxY = (int)ceilf(worldMax.y / TILE_SIZE) + 1;

    // clamp ke batas map yang valid
    if (range.minX < 0)
        range.minX = 0;
    if (range.minY < 0)
        range.minY = 0;
    if (range.maxX > tilesonMap->width)
        range.maxX = tilesonMap->width;
    if (range.maxY > tilesonMap->height)
        range.maxY = tilesonMap->height;

    return range;
}
