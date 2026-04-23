/**
 * @file player.cpp
 * @brief Implementasi dari Player System Module
 *
 * Implementasi dari class Player yang dideklarasikan di player.h
 * Handle movement, collision detection, animasi, camera follow, dan interaksi.
 */

#include "../include/mapLogic.h"
#include "../include/player.h"
#include "../include/debug.h"
#include "../include/map.h"
#include "../include/animation.h"
#include "../include/input.h"
#include "../lib/raylib/include/raymath.h"
#include <cmath>

/*==============================================================================
 * Global Variables
 *==============================================================================*/

/** Global instance player — diakses file lain via extern */
Player PlayerInstance;

/*==============================================================================
 * Private Helper Methods
 *==============================================================================*/

/**
 * @brief Dapetin hitbox player di posisi tertentu
 * @param position Posisi yang mau dicek
 * @return Rectangle hitbox dengan offset yang udah di-apply
 */
Rectangle Player::GetPlayerHitboxAtPosition(Vector2 position)
{
    return BuildHitbox(position, HitboxOffsetX, HitboxOffsetY, HitboxWidth, HitboxHeight);
}

/*==============================================================================
 * Initialization
 *==============================================================================*/

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
void Player::Init(GameState *state, const char *spawnObjectName)
{
    State = state;

    // ================================================================
    // Step 1: Inisialisasi Satu Kali (Resource Global & Stats)
    // ================================================================
    if (!isInitialized)
    {
        // Load texture karakter
        LoadTileTexture(TEXTURE_KNIGHT, "texture/knight.png");
        LoadTileTexture(TEXTURE_ITEMS,  "texture/test.png");

        // init health
        MaxHealth = 100.0f;
        Health = MaxHealth;

        // init mana
        MaxMana = 100.0f;
        Mana = MaxMana;
        ManaRegenTimer = 0.0f;

        // Initialize default hotbar items
        Hotbar[0] = {ITEM_WEAPON, "Iron Sword", 1, 10, 0, 6, 4};
        Hotbar[1] = {ITEM_WEAPON, "Wooden Bow", 1, 5, 0, 8, 4};
        Hotbar[2] = {ITEM_POTION, "Health Potion", 3, 0, 20, 7, 8};
        Hotbar[3] = {ITEM_POTION, "Mana Bread", 5, 0, 15, 10, 8};

        isInitialized = true;
        TraceLog(LOG_INFO, "Player: Global resources and stats initialized");
    }

    // ================================================================
    // Step 2: Inisialisasi Setiap Pindah Map (Spawn & Collision)
    // ================================================================

    // Inisialisasi state animasi player (reset state ke IDLE pas pindah map)
    Anim.position = {0.0f, 0.0f};
    Anim.state = IDLE;
    Anim.direction = DOWN;
    Anim.frame = 0;
    Anim.frameTime = 0.0f;
    Anim.frameSpeed = 0.5f;
    Anim.walkFrameIndex = 0;
    Anim.isAttacking = false;
    Anim.isDead = false;

    // reset collision cache biar aman kalau nanti map di-reload
    CollisionRects.clear();
    CollisionPolygons.clear();

    // Safety check kalau map belum keload
    if (tilesonMap == nullptr)
    {
        Position = {0.0f, 0.0f};
        Anim.position = Position;
        TraceLog(LOG_ERROR, "Player: tilesonMap is null during Init()");
        return;
    }

    // Ambil spawn point dari object layer Tiled
    Vector2 spawnPos;
    if (spawnObjectName != nullptr &&
        spawnObjectName[0] != '\0' &&
        TiledHelperFunction.TryGetObjectPositionByName(spawnObjectName, spawnPos))
    {
        Position = spawnPos;
        TraceLog(LOG_INFO, "Player: Spawn point '%s' found at (%.1f, %.1f)", spawnObjectName, Position.x, Position.y);
    }
    else if (TiledHelperFunction.TryGetObjectPositionByName(SPAWN_OBJECT_NAME, spawnPos))
    {
        Position = spawnPos;
        TraceLog(LOG_INFO, "Player: Default spawn point found at (%.1f, %.1f)", Position.x, Position.y);
    }
    else
    {
        // Fallback kalau object spawn belum ada di Tiled
        Position = {
            ((float)tilesonMap->width * TILE_SIZE) / 2.0f,
            ((float)tilesonMap->height * TILE_SIZE) / 2.0f};
        TraceLog(LOG_WARNING, "Player: Spawn object '%s' not found, using default position",
                 (spawnObjectName != nullptr && spawnObjectName[0] != '\0') ? spawnObjectName : SPAWN_OBJECT_NAME);
    }

    // Sync posisi animasi dengan posisi player
    Anim.position = Position;

    // Ambil semua collision object dari object layer Tiled
    TiledHelper::CollisionResult collision;
    if (TiledHelperFunction.TryGetCollisionByLayerName(COLLISION_LAYER_NAME, collision))
    {
        CollisionRects = collision.rects;
        CollisionPolygons = collision.polygons;
    }

    TraceLog(LOG_INFO, "Player: Loaded %d collision rects", (int)CollisionRects.size());
    TraceLog(LOG_INFO, "Player: Loaded %d collision polygons", (int)CollisionPolygons.size());
}

/*==============================================================================
 * Update & Input Handling
 *==============================================================================*/

// ================================================================
// Update()
// Handle input keyboard dan movement player per frame.
// Normalisasi velocity biar diagonal gak lebih cepet dari cardinal.
// Cek collision sebelum apply posisi baru.
// ================================================================
void Player::Update()
{
    // Poll input di awal frame
    InputInstance.PollInput();
    InputInstance.UpdateState();

    // ===== Revive (R) — bisa dipanggil kapan saja, bahkan saat dead =====
    if (InputInstance.IsRevive())
    {
        HandleRevive();
        return;
    }

    // Kalau player dead, skip semua input selain revive
    if (Anim.isDead)
        return;

    if (Health <= 0)
    {
        Health = 0;
        UpdatePlayerDeath(Anim);
        return;
    }

    // --- Mana Regeneration ---
    // Timer tetap berjalan meskipun sedang attack (biar cooldown gak terasa delay karena animasi)
    if (ManaRegenTimer > 0)
    {
        ManaRegenTimer -= GetFrameTime();
    }
    else
    {
        if (Mana < MaxMana)
        {
            Mana += ManaRegenRate * GetFrameTime();
            if (Mana > MaxMana)
                Mana = MaxMana;
        }
    }

    // --- Health Test (DEBUG) ---
    if (InputInstance.IsTestLoseHP())
    {
        Health -= 10.0f;
        if (Health < 0)
            Health = 0;
        TraceLog(LOG_INFO, "PLAYER: Test Health Decrease (%.1f)", Health);
    }

    // --- Left Click — context action (attack / potion / equip) ---
    if (InputInstance.IsLeftClickPressed() && !Anim.isAttacking)
    {
        HandleAction();
        // kalau jadi attack, skip movement frame ini
        if (Anim.isAttacking)
            return;
    }

    // ===== Go Back (B) — kembali ke map sebelumnya =====
    if (InputInstance.IsGoBack())
    {
        pendingSwitchMap = false; // cancel kalau ada pending switch map
        pendingGoBack = true;     // trigger go back di Tick()
        return;
    }

    // Kalau lagi attack, skip movement
    if (Anim.isAttacking)
        return;

    // ===== Interact (E) =====
    if (InputInstance.IsInteract())
    {
        // TODO: implementasi interact dengan object/NPC di depan player
        TraceLog(LOG_INFO, "PLAYER: Interact triggered");
    }

    // ===== Movement (Arrow/WASD) =====
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

    // Set animation state berdasarkan movement
    if (moving)
        Anim.state = WALK;
    else
        Anim.state = IDLE;

    // Normalisasi biar diagonal gak lebih cepet dari cardinal
    float Length = sqrtf(Velocity.x * Velocity.x + Velocity.y * Velocity.y);
    if (Length != 0)
    {
        Velocity.x /= Length;
        Velocity.y /= Length;
    }

    // Apply movement dengan collision check
    Vector2 NewPos = {
        Position.x + Velocity.x * Speed,
        Position.y + Velocity.y * Speed};

    if (CanMove(NewPos))
        Position = NewPos;

    // Sync posisi animasi dengan posisi player
    Anim.position = Position;

    // Cek interaksi dengan pintu + raycast
    RayCasting();
    CheckDoorInteraction();
    CheckPropInteraction();
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

/*==============================================================================
 * Main Update Loop & Frustum Culling
 *==============================================================================*/

// ================================================================
// Tick()
// Wrapper logic player per frame — dipanggil dari UpdateLogicAll().
// Urutan: update input/movement → update animasi → update camera
// ================================================================
void Player::Tick()
{
    // Step 1: Update movement dan input
    Update();

    // Step 2: Handle pending go back (kembali ke map sebelumnya)
    if (PlayerInstance.pendingGoBack)
    {
        PlayerInstance.pendingGoBack = false;
        GoBack();
        return;
    }

    // Step 3: Handle pending switch map (pindah map lewat pintu)
    if (PlayerInstance.pendingSwitchMap)
    {
        PlayerInstance.pendingSwitchMap = false;
        SwitchMap(PlayerInstance.pendingMapPath.c_str(), PlayerInstance.pendingDoorName.c_str());
    }

    // Step 4: Update animasi berdasarkan delta time
    UpdateAnimation(Anim, GetFrameTime());

    // Step 5: Update camera follow player
    PlayerCamera();
}

/*==============================================================================
 * Camera System
 *==============================================================================*/

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

    // Zoom otomatis kalau map <= MinMapTileZoom biar map ngisi viewport
    // Kalau map lebih gede, pake FixedZoom — ubah nilai ini buat adjust
    const int MinMapTileZoom = 15;
    float AutoZoom = (float)GameScreenWidth / (MinMapTileZoom * TILE_SIZE);
    float FixedZoom = 2.0f;
    const float CameraZoom = (tilesonMap->width <= MinMapTileZoom || tilesonMap->height <= MinMapTileZoom)
                                 ? AutoZoom
                                 : FixedZoom;

    // Kalau debug mode off, pakai zoom yang udah dihitung
    if (!isDebugMode)
        camera.zoom = CameraZoom;

    // Camera target = tengah player
    camera.target.x = PlayerInstance.GetPosition().x + (TILE_SIZE / 2.0f);
    camera.target.y = PlayerInstance.GetPosition().y + (TILE_SIZE / 2.0f);

    // Hitung half viewport dalam world space
    float halfW = (GameScreenWidth / 2.0f) / camera.zoom;
    float halfH = (GameScreenHeight / 2.0f) / camera.zoom;

    // Clamp camera target ke world bounds
    // Camera berhenti ngikutin player kalau udah di tepi map
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

/*==============================================================================
 * Collision & Movement
 *==============================================================================*/

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
    // Step 1: Build hitbox di posisi baru
    Rectangle hitbox = GetPlayerHitboxAtPosition(newPosition);

    // Step 2: Cek world bounds (batas luar map)
    if (!IsWithinWorldBounds(hitbox, tilesonMap->width * TILE_SIZE, tilesonMap->height * TILE_SIZE))
        return false;

    // Step 3: Cek collision dengan rectangle obstacles
    if (CheckCollisionAgainstRects(hitbox, CollisionRects))
        return false;

    // Step 4: Cek collision dengan polygon obstacles
    if (CheckCollisionAgainstPolygons(hitbox, CollisionPolygons))
        return false;

    return true; // Aman, gak nabrak apa-apa
}

/**
 * @brief Update arah ray berdasarkan posisi mouse dan cast ke prop objects
 * @note Origin ray dari tengah hitbox player, bukan pojok kiri atas sprite
 *       Mouse di-convert ke world space dulu sebelum dihitung direction-nya
 *       Hasil hit disimpan di LastHit, dibaca oleh CheckPropInteraction()
 */
void Player::RayCasting()
{
    Vector2 playerCenter = {
        Position.x + HitboxOffsetX + HitboxWidth / 2,
        Position.y + HitboxOffsetY + HitboxHeight / 2};

    // convert mouse ke world space
    Vector2 mouseWorld = GetScreenToWorld2D(GetVirtualMousePosition(State), camera);

    // hitung direction dari player ke mouse
    Vector2 aimDir = Vector2Normalize(Vector2Subtract(mouseWorld, playerCenter));

    // filter prop objects
    std::vector<MapObject> propObjects;
    for (auto &obj : tilesonMap->Objects)
    {
        if (obj.layerName == OBJECT_LAYER_NAME)
            propObjects.push_back(obj);
    }

    LastHit = Ray.Cast(playerCenter, aimDir, INTERACT_RANGE, propObjects);
}
/*==============================================================================
 * Action Handlers
 *==============================================================================*/

// ================================================================
// HandleAction()
// Resolve apa yang terjadi saat input aksi (misal: Left Click)
// dilakukan berdasarkan context:
// - Inventori terbuka → equip/unequip item
// - Slot senjata (1/2) → attack
// - Slot potion (3/4) → minum potion
// ================================================================
void Player::HandleAction(void)
{
    PlayerAction action = InputInstance.ResolveAction();

    switch (action)
    {
    case ACTION_ATTACK:
        if (Mana >= AttackManaCost)
        {
            UpdatePlayerAttack(Anim);
            Mana -= AttackManaCost;
            ManaRegenTimer = ManaRegenDelay; // Reset timer ke 2 detik
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
        if (slotIdx >= 0 && slotIdx < 4)
        {
            if (Hotbar[slotIdx].type == ITEM_POTION && Hotbar[slotIdx].amount > 0)
            {
                UsePotion(slotIdx);
                TraceLog(LOG_INFO, "PLAYER: Drink potion! %s (slot %d)", Hotbar[slotIdx].name.c_str(), (int)InputInstance.GetActiveSlot());
            }
            else
            {
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

    // Cek apakah item mengandung kata "Mana" untuk mendeteksi tipe heal
    if (Hotbar[slotIndex].name.find("Mana") != std::string::npos)
    {
        // Apply mana heal
        Mana += Hotbar[slotIndex].healValue;
        if (Mana > MaxMana)
            Mana = MaxMana;
        TraceLog(LOG_INFO, "PLAYER: Healed Mana by %d! Current: %.1f", Hotbar[slotIndex].healValue, Mana);
    }
    else
    {
        // Apply health heal
        Health += Hotbar[slotIndex].healValue;
        if (Health > MaxHealth)
            Health = MaxHealth;
        TraceLog(LOG_INFO, "PLAYER: Healed Health by %d! Current: %.1f", Hotbar[slotIndex].healValue, Health);
    }

    // Consume item
    Hotbar[slotIndex].amount--;
    if (Hotbar[slotIndex].amount <= 0)
    {
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

/*==============================================================================
 * Door Interaction
 *==============================================================================*/

/**
 * @brief Cek interaksi dengan pintu dan trigger switch map
 * @note Dipanggil tiap frame, kalo player nabrak pintu dan tekan E
 */
void Player::CheckDoorInteraction(void)
{
    Rectangle playerHitbox = GetPlayerHitboxAtPosition(Position);

    // Ambil semua object dengan type "pass" (pintu)
    std::vector<MapObject *> doors = TiledHelperFunction.GetObjectsByType(DOOR_TYPE_OBJECT_NAME);

    for (const auto &door : doors)
    {
        // Cek apakah player nabrak pintu
        if (!CheckCollisionRecs(playerHitbox, door->bounds))
            continue;

        // Kalo nabrak tapi gak tekan E, gak terjadi apa-apa
        if (!InputInstance.IsInteract())
            return;

        // Cek properti "target_map" (map tujuan)
        auto mapIt = door->properties.find("target_map");
        if (mapIt == door->properties.end())
        {
            TraceLog(LOG_WARNING, "Door '%s' has no target_map property", door->name.c_str());
            return;
        }

        // Cek properti "target_door" (spawn point di map tujuan)
        auto doorIt = door->properties.find("target_door");
        if (doorIt == door->properties.end())
        {
            TraceLog(LOG_WARNING, "Door '%s' has no target_door property", door->name.c_str());
            return;
        }

        // Set pending switch map (dieksekusi di Tick())
        std::string targetMap = mapIt->second.getValue<std::string>();
        std::string targetDoor = doorIt->second.getValue<std::string>();

        pendingSwitchMap = true;
        pendingMapPath = targetMap;
        pendingDoorName = targetDoor;
        return;
    }
}

/**
 * @brief Entry point semua interaksi prop berdasarkan hasil raycasting
 * @note Hanya trigger kalau LastHit.hit true dan player tekan tombol interact
 *       Type object diambil dari field type di Tiled — harus koordinasi sama tim
 *       buat naming convention (chest, npc, dll)
 */
void Player::CheckPropInteraction(void)
{
    if (!LastHit.hit)
        return;
    if (!InputInstance.IsInteract())
        return;

    const std::string &type = LastHit.object->type;
    TraceLog(LOG_INFO, "Interacting with: '%s' (type: %s)", LastHit.object->name.c_str(), type.c_str());

    if (type == CHEST_TYPE_OBJECT_NAME)
    {
        TraceLog(LOG_INFO, "Opening chest: '%s'", LastHit.object->name.c_str());
    }
    else if (type == "npc")
    {
        // TODO: trigger dialog
    }
    else
    {
        TraceLog(LOG_WARNING, "Unknown prop type: %s", type.c_str());
    }
}