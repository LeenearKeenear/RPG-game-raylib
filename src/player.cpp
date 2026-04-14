#include "../include/player.h"
#include "../include/debug.h"
#include "../include/map.h"
#include "../include/animation.h"
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

    // reset collision cache biar aman kalau nanti map di-reload
    CollisionRects.clear();
    CollisionPolygons.clear();
    WorldBoundaryPolygon.clear();

    // safety check kalau map belum keload
    if (tilesonMap == nullptr)
    {
        Position = {0.0f, 0.0f};
        TraceLog(LOG_ERROR, "Player: tilesonMap is null during Init()");
        return;
    }

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
        // posisi fallback dipusatkan ke tengah map dalam pixel
        Position = {
            (((float)tilesonMap->width * TILE_SIZE) / 2.0f) + TILE_SIZE * 3,
            ((float)tilesonMap->height * TILE_SIZE) / 2.0f};
        TraceLog(LOG_WARNING, "Player: Spawn object '%s' not found, using default position", SPAWN_OBJECT_NAME);
    }

    /// ambil semua collision object dari object layer Tiled
    // rectangle disimpan ke CollisionRects, polygon disimpan ke CollisionPolygons
    std::vector<MapObject> collisionObjs = TilesonGetObjectsByLayerName(COLLISION_LAYER_NAME);
    for (auto &obj : collisionObjs)
    {
        if (obj.hasPolygon)
            CollisionPolygons.push_back(obj.polygonPoints);
        else
            CollisionRects.push_back(obj.bounds);
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
// Dipanggil dari RenderEntities() setelah RenderMap().
// ================================================================
void Player::Render(void)
{
    RenderTilePNG(Position.x, Position.y, TILE_PLAYER_NEW, 0.0f, TEXTURE_KNIGHT);
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
bool Player::CanMove(Vector2 NewPos)
{
    Rectangle playerBox = {
        NewPos.x + HitboxOffsetX,
        NewPos.y + HitboxOffsetY,
        HitboxWidth,
        HitboxHeight};

    // titik-titik penting player box buat cek polygon collision
    Vector2 playerPoints[4] = {
        {playerBox.x, playerBox.y},
        {playerBox.x + playerBox.width, playerBox.y},
        {playerBox.x, playerBox.y + playerBox.height},
        {playerBox.x + playerBox.width, playerBox.y + playerBox.height}};

    // helper point-in-polygon sederhana (ray casting)
    auto IsPointInsidePolygon = [](Vector2 point, const std::vector<Vector2> &polygonPoints) -> bool
    {
        bool inside = false;
        int pointCount = (int)polygonPoints.size();

        if (pointCount < 3)
            return false;

        for (int i = 0, j = pointCount - 1; i < pointCount; j = i++)
        {
            const Vector2 &pi = polygonPoints[i];
            const Vector2 &pj = polygonPoints[j];

            bool intersect = ((pi.y > point.y) != (pj.y > point.y)) &&
                             (point.x < (pj.x - pi.x) * (point.y - pi.y) / ((pj.y - pi.y) + 0.00001f) + pi.x);

            if (intersect)
                inside = !inside;
        }

        return inside;
    };
    // ============================================================
    // 1. collision rectangle biasa dari object layer Tiled
    // setiap rectangle object di layer collision dianggap solid
    // ============================================================
    for (auto &rect : CollisionRects)
    {
        if (CheckCollisionRecs(playerBox, rect))
            return false;
    }

    // ============================================================
    // 2. collision polygon dari object layer Tiled
    // kalau salah satu titik player masuk ke polygon collision,
    // gerakan diblok
    // ============================================================
    for (auto &polygon : CollisionPolygons)
    {
        for (int i = 0; i < 4; i++)
        {
            if (IsPointInsidePolygon(playerPoints[i], polygon))
                return false;
        }
    }

    // ============================================================
    // 3. world bound rectangle dari ukuran map
    // custom world boundary polygon sementara tidak dipakai,
    // karena hasil runtime nunjukin polygon seperti OffmapBoundary
    // lebih cocok diperlakukan sebagai collision polygon.
    //
    // Jadi rule batas dunia sekarang disederhanakan:
    // - area terlarang/non-playable ditangani collision object
    // - batas luar map ditangani rectangle ukuran map
    // ============================================================
    float MapW = (float)tilesonMap->width * TILE_SIZE;
    float MapH = (float)tilesonMap->height * TILE_SIZE;

    if (playerBox.x < 0.0f)
        return false;
    if (playerBox.y < 0.0f)
        return false;
    if (playerBox.x + playerBox.width > MapW)
        return false;
    if (playerBox.y + playerBox.height > MapH)
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
// Urutan: update input/movement → update camera
// ================================================================
void Player::Tick(void)
{
    Update();
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
