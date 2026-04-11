#include "../include/player.h"
#include "../include/debug.h"
#include "../include/map.h"
#include <cmath>

// global instance player — diakses file lain via extern
Player PlayerInstance;

// ================================================================
// Init()
// Inisialisasi player: load texture, ambil spawn point dari
// object layer Tiled (nama object: SPAWN_OBJECT_NAME "spawn"),
// dan load semua collision rectangles dari object layer
// (type: COLLISION_LAYER_NAME "collision")
//
// Cara kerja:
// 1. Load texture character dari file png
// 2. Cari object "spawn" di tilesonMap->Objects → set Position
// 3. Ambil semua object type "collision" → simpen di CollisionRects
// ================================================================
void Player::Init()
{
    // load texture character
    // TODO: path texture disesuaiin sama asset yang ada
    LoadTileTexture(TEXTURE_KNIGHT, "texture/Knight.png");

    // ambil spawn point dari object layer Tiled
    MapObject *spawnObj = TilesonGetObjectByName(SPAWN_OBJECT_NAME);
    if (spawnObj != nullptr)
    {
        // bounds.x dan bounds.y itu posisi object di Tiled (dalam pixel)
        Position = {spawnObj->bounds.x, spawnObj->bounds.y};
        TraceLog(LOG_INFO, "Player: Spawn point found at (%.1f, %.1f)", Position.x, Position.y);
    }
    else
    {
        // fallback kalau object spawn belum ada di Tiled
        Position = {160.0f, 160.0f};
        TraceLog(LOG_WARNING, "Player: Spawn object '%s' not found, using default position", SPAWN_OBJECT_NAME);
    }

    // ambil semua collision rectangles dari object layer Tiled
    // setiap rectangle object di layer "collision" dianggap solid/blocked
    std::vector<MapObject> collisionObjs = TilesonGetObjectsByType(COLLISION_LAYER_NAME);
    for (auto &obj : collisionObjs)
        CollisionRects.push_back(obj.bounds);

    TraceLog(LOG_INFO, "Player: Loaded %d collision rects", (int)CollisionRects.size());
}

// ================================================================
// Update()
// Handle input keyboard dan movement player per frame.
// Normalisasi velocity biar diagonal gak lebih cepet.
// Cek collision sebelum apply posisi baru.
// ================================================================
void Player::Update()
{
    Velocity = {0, 0};

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        Velocity.y -= 1;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        Velocity.y += 1;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        Velocity.x -= 1;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        Velocity.x += 1;

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
}

// ================================================================
// Render()
// Render sprite player di posisi world saat ini.
// Dipanggil dari RenderEntities() setelah TilesonRender().
// ================================================================
void Player::Render()
{
    RenderTilePNG(Position.x, Position.y, TILE_PLAYER_NEW, 0.0f, TEXTURE_KNIGHT);
}

// ================================================================
// CanMove()
// Cek apakah posisi baru player (NewPos) nabrak salah satu
// collision rectangle dari object layer Tiled.
//
// Collision box player diasumsiin 1 tile (TileSize x TileSize).
// Return false kalau nabrak, true kalau aman.
// ================================================================
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
// PlayerCamera()
// Handle camera follow player dengan clamp ke world bounds.
//
// Cara kerja:
// 1. Zoom fixed di CameraZoom (default 1.0f), ubah nilai ini buat adjust
// 2. Camera target selalu nge-follow tengah player
// 3. Camera di-clamp biar gak keluar dari world bounds
// ================================================================
void Player::PlayerCamera(void)
{
    // ambil ukuran world dari tilesonMap
    float MapW = (float)tilesonMap->width * TILE_SIZE;
    float MapH = (float)tilesonMap->height * TILE_SIZE;

    // zoom otomatis kalau map <= MinMapTileZoom, otherwise 1.0f (adjust manual)
    // MinMapTileZoom = ukuran map minimum yang dijamin ngisi viewport
    const int MinMapTileZoom = 15;
    float AutoZoom = (float)GameScreenWidth / (MinMapTileZoom * TILE_SIZE);
    float FixedZoom = 2.0f; // ini yang diganti
    const float CameraZoom = (tilesonMap->width <= MinMapTileZoom || tilesonMap->height <= MinMapTileZoom) ? AutoZoom : FixedZoom;

    // kalau debug mode off, pakai zoom fixed
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
    if (camera.target.x < halfW)
        camera.target.x = halfW;
    if (camera.target.y < halfH)
        camera.target.y = halfH;
    if (camera.target.x > MapW - halfW)
        camera.target.x = MapW - halfW;
    if (camera.target.y > MapH - halfH)
        camera.target.y = MapH - halfH;
}

void Player::Tick()
{
    Update();
    PlayerCamera();
}