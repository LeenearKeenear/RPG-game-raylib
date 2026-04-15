#include "../include/player.h"
#include "../include/debug.h"
#include "../include/map.h"
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
// (type: COLLISION_LAYER_NAME "collision")
//
// Cara kerja:
// 1. Load texture character dari file png
// 2. Cari object "spawn" di tilesonMap->Objects → set Position
// 3. Ambil semua object type "collision" → simpen di CollisionRects
// ================================================================
void Player::Init(void)
{
    // Load texture knight dari file png untuk animasi
    LoadTileTexture(TEXTURE_KNIGHT, "texture/Knight.png");
    CharTexture = TexturesMap[TEXTURE_KNIGHT];

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
// Normalisasi velocity biar diagonal gak lebih cepet dari cardinal.
// Cek collision sebelum apply posisi baru.
// ================================================================
void Player::Update(void)
{
    // --- DETEKSI REVIVE (Hidup Kembali) ---
    // Tekan R untuk hidup lagi setelah mati (hanya untuk keperluan testing)
    if (isDead && IsKeyPressed(KEY_R)) {
        isDead = false;
        currentState = IDLE;
        return;
    }

    // Kalau sudah mati, tidak bisa ngapa-ngapain lagi (semua input jalan/serang diblokir)
    if (isDead) return;

    Velocity = {0, 0};

    // --- DETEKSI INPUT SERANGAN ---
    // Gunakan Spasi untuk menyerang
    if (IsKeyPressed(KEY_SPACE) && !isAttacking) {
        currentState = ATTACK;
        frame = 0;
        frameTime = 0.0f;
        isAttacking = true;
    }

    // --- DETEKSI MATI (Tombol K untuk test mati) ---
    if (IsKeyPressed(KEY_K)) {
        currentState = DEAD;
        frame = 0;
        isDead = true;
        return;
    }

    // Jika sedang serang, jangan jalankan proses movement
    if (!isAttacking)
    {
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

        // --- ANIMATION STATE LOGIC (Walk/Idle) ---
        if (Velocity.x < 0) currentDir = LEFT;
        else if (Velocity.x > 0) currentDir = RIGHT;
        else if (Velocity.y < 0) currentDir = UP;
        else if (Velocity.y > 0) currentDir = DOWN;

        if (Length != 0) currentState = WALK;
        else currentState = IDLE;
    }

    // --- FRAME UPDATE LOGIC ---
    frameTime += GetFrameTime();
    
    if (currentState == IDLE) {
        frameSpeed = 0.5f; // Animasi idle lebih lambat
        if (frameTime >= frameSpeed) {
            frame = (frame + 1) % 2; // Frame 0 & 1 untuk idle
            frameTime = 0;
        }
    } 
    else if (currentState == WALK) {
        frameSpeed = 0.12f; // Animasi jalan lebih cepat
        if (frameTime >= frameSpeed) {
            walkFrameIndex = (walkFrameIndex + 1) % 4;
            int walkFrames[4] = {1, 2, 1, 3}; // Standing -> Left -> Standing -> Right
            frame = walkFrames[walkFrameIndex];
            frameTime = 0;
        }
    }
    else if (currentState == ATTACK) {
        frameSpeed = 0.12f; // Animasi serang sedikit lebih cepat biar mulus
        if (frameTime >= frameSpeed) {
            frame++;
            frameTime = 0;
            int maxAttackFrames = 1;

            // Jika animasi serangan selesai
            if (frame > maxAttackFrames) {
                isAttacking = false;
                currentState = IDLE;
                frame = 0;
            }
        }
    }
}

// ================================================================
// Render()
// Render sprite player di posisi world saat ini.
// Dipanggil dari RenderEntities() setelah RenderMap().
// ================================================================
void Player::Render(void)
{
    // Ambil source rectangle dari spritesheet Knight.png
    // Row ditentukan oleh arah (Direction enum), Column oleh frame saat ini
    int row = (int)currentDir;
    int col = frame;

    // Override khusus jika sedang menyerang
    if (currentState == ATTACK) {
        if (currentDir == LEFT || currentDir == RIGHT) {
            // Kiri/Kanan: Tile 7, 8, 6, 5 (1-based) -> 6, 7, 5, 4 (0-based)
            int attackFrames[4] = {6, 7};
            col = attackFrames[frame % 4]; 
        } else {
            // Atas/Bawah: Tile 5, 6 (1-based) -> 4, 5 (0-based)
            int attackFrames[2] = {4, 5};
            col = attackFrames[frame % 2];
        }
    }

    // Override khusus jika sedang dalam status mati
    if (currentState == DEAD) {
        row = 4; // Berdasarkan file animation.cpp
        col = 0;
    }

    Rectangle src = GetFrame(col, row);
    
    // Gambar player menggunakan texture yang sudah di-load
    DrawTextureRec(CharTexture, src, Position, WHITE);
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
    if (camera.target.x < halfW)
        camera.target.x = halfW;
    if (camera.target.y < halfH)
        camera.target.y = halfH;
    if (camera.target.x > MapW - halfW)
        camera.target.x = MapW - halfW;
    if (camera.target.y > MapH - halfH)
        camera.target.y = MapH - halfH;
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
    range.maxX = (int)ceilf(worldMax.x  / TILE_SIZE) + 1;
    range.maxY = (int)ceilf(worldMax.y  / TILE_SIZE) + 1;

    // clamp ke batas map yang valid
    if (range.minX < 0)                       range.minX = 0;
    if (range.minY < 0)                       range.minY = 0;
    if (range.maxX > tilesonMap->width)       range.maxX = tilesonMap->width;
    if (range.maxY > tilesonMap->height)      range.maxY = tilesonMap->height;

    return range;
}
