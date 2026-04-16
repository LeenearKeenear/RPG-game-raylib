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

Rectangle Player::GetPlayerHitboxAtPosition(Vector2 position)
{
    return BuildHitbox(position, HitboxOffsetX, HitboxOffsetY, HitboxWidth, HitboxHeight);
}

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
void Player::Init(const char *spawnObjectName)
{
    // TODO: path texture disesuaiin sama asset yang ada
    LoadTileTexture(TEXTURE_KNIGHT, "texture/Knight.png");

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
        // fallback kalau object spawn belum ada di Tiled
        // posisi fallback dipusatkan ke tengah map dalam pixel
        Position = {
            ((float)tilesonMap->width * TILE_SIZE) / 2.0f,
            ((float)tilesonMap->height * TILE_SIZE) / 2.0f};
        TraceLog(LOG_WARNING, "Player: Spawn object '%s' not found, using default position",
                 (spawnObjectName != nullptr && spawnObjectName[0] != '\0') ? spawnObjectName : SPAWN_OBJECT_NAME);
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
    if (Anim.isDead)
        return;

    // --- Kill (K) — debug: player langsung mati ---
    if (InputInstance.IsKill())
    {
        UpdatePlayerDeath(Anim);
        return;
    }

    // --- Left Click — context action (attack / potion / equip) ---
    if (InputInstance.IsLeftClickPressed() && !Anim.isAttacking)
    {
        HandleSpaceAction();
        // kalau jadi attack, skip movement frame ini
        if (Anim.isAttacking)
            return;
    }

    // --- Go Back (B) — kembali ke map sebelumnya ---
    if (InputInstance.IsGoBack())
    {
        pendingSwitchMap = false; // cancel kalau ada pending
        // GoBack dipanggil via pending juga biar aman
        pendingGoBack = true;
        return;
    }

    // kalau lagi attack, skip movement
    if (Anim.isAttacking)
        return;

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

    CheckDoorInteraction();
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
        UpdatePlayerAttack(Anim);
        TraceLog(LOG_INFO, "PLAYER: Attack! (slot %d)", (int)InputInstance.GetActiveSlot());
        break;

    case ACTION_DRINK_POTION:
        // TODO: implementasi efek potion (heal, buff, dll)
        TraceLog(LOG_INFO, "PLAYER: Drink potion! (slot %d)", (int)InputInstance.GetActiveSlot());
        break;

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
    Rectangle hitbox = GetPlayerHitboxAtPosition(newPosition);

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

void Player::CheckDoorInteraction(void)
{
    Rectangle playerHitbox = GetPlayerHitboxAtPosition(Position);

    std::vector<MapObject> doors = TiledHelperFunction.GetObjectsByType(DOOR_TYPE_OBJECT_NAME);

    for (const auto &door : doors)
    {
        if (!CheckCollisionRecs(playerHitbox, door.bounds))
            continue;

        if (!IsKeyPressed(KEY_E))
            return;

        auto mapIt = door.properties.find("target_map");
        if (mapIt == door.properties.end())
        {
            TraceLog(LOG_WARNING, "Door '%s' has no target_map property", door.name.c_str());
            return;
        }

        auto doorIt = door.properties.find("target_door");
        if (doorIt == door.properties.end())
        {
            TraceLog(LOG_WARNING, "Door '%s' has no target_door property", door.name.c_str());
            return;
        }

        std::string targetMap = mapIt->second.getValue<std::string>();
        std::string targetDoor = doorIt->second.getValue<std::string>();

        pendingSwitchMap = true;
        pendingMapPath = targetMap;
        pendingDoorName = targetDoor;
        return;
    }
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
    if (PlayerInstance.pendingGoBack)
    {
        PlayerInstance.pendingGoBack = false;
        GoBack();
        return;
    }

    if (PlayerInstance.pendingSwitchMap)
    {
        PlayerInstance.pendingSwitchMap = false;
        SwitchMap(PlayerInstance.pendingMapPath.c_str(), PlayerInstance.pendingDoorName.c_str());
    }

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
