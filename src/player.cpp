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
// 4. Setup hotbar default (2 weapon + 2 potion)
// ================================================================
void Player::Init(void)
{
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

    // ---- Setup hotbar default ----
    // Slot 1 & 2 = weapon, Slot 3 & 4 = potion
    Hotbar[0] = {SLOT_WEAPON, "Sword",          false};
    Hotbar[1] = {SLOT_WEAPON, "Bow",            false};
    Hotbar[2] = {SLOT_POTION, "Health Potion",  false};
    Hotbar[3] = {SLOT_POTION, "Mana Potion",    false};

    SelectedHotbarSlot = 0;
    State = PLAYER_IDLE;
    bIsAlive = true;
    bInventoryOpen = false;
    bMapOpen = false;

    TraceLog(LOG_INFO, "Player: Key bindings initialized");
    TraceLog(LOG_INFO, "  WASD/Arrow = Move (or navigate inventory)");
    TraceLog(LOG_INFO, "  E          = Interact");
    TraceLog(LOG_INFO, "  I          = Toggle Inventory");
    TraceLog(LOG_INFO, "  M          = Toggle Map");
    TraceLog(LOG_INFO, "  1-4        = Select Hotbar Slot");
    TraceLog(LOG_INFO, "  Space      = Action (context-sensitive)");
    TraceLog(LOG_INFO, "  K          = Test Death");
    TraceLog(LOG_INFO, "  R          = Test Revive");
}

// ================================================================
// GetHotbarSlot()
// Getter untuk info slot hotbar berdasarkan index (0-3)
// ================================================================
HotbarSlot Player::GetHotbarSlot(int index)
{
    if (index >= 0 && index < 4)
        return Hotbar[index];

    return {SLOT_WEAPON, "Empty", true};
}

// ================================================================
// HandleInput()
// Master input handler — dipanggil tiap frame dari Tick().
// Distribusi input ke handler masing-masing.
//
// Prioritas input:
// 1. Kalau player mati → hanya boleh R (revive)
// 2. Kalau inventory/map terbuka → WASD jadi navigasi, Space jadi equip
// 3. Normal gameplay → semua key aktif
// ================================================================
void Player::HandleInput(void)
{
    // kalau mati, hanya bisa revive
    if (!bIsAlive)
    {
        HandleTestRevive();
        return;
    }

    // kalau sedang dalam aksi (attack/potion), tunggu selesai
    if (State == PLAYER_ATTACKING || State == PLAYER_DRINKING_POTION)
    {
        ActionTimer -= GetFrameTime();
        if (ActionTimer <= 0.0f)
        {
            State = PLAYER_IDLE;
            ActionTimer = 0.0f;
            TraceLog(LOG_INFO, "Player: Action finished, back to IDLE");
        }
        return; // block input selama aksi berlangsung
    }

    // toggle inventory dan map (bisa kapan aja selama hidup)
    HandleInventory();
    HandleMap();

    // kalau inventory terbuka, WASD jadi navigasi inventory
    // dan Space jadi equip/unequip
    if (bInventoryOpen)
    {
        HandleActionKey(); // Space = equip/unequip di inventory
        return;            // block movement & hotbar di inventory mode
    }

    // kalau map terbuka, block semua input selain close map
    if (bMapOpen)
        return;

    // normal gameplay input
    HandleMovement();
    HandleHotbar();
    HandleActionKey();
    HandleInteract();
    HandleTestDeath();
    HandleTestRevive();
}

// ================================================================
// HandleMovement()
// Handle WASD/Arrow key untuk movement player.
// Normalisasi velocity biar diagonal gak lebih cepet dari cardinal.
// Set state ke MOVING kalau ada input, IDLE kalau tidak.
// ================================================================
void Player::HandleMovement(void)
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
        State = PLAYER_MOVING;
    }
    else
    {
        if (State == PLAYER_MOVING)
            State = PLAYER_IDLE;
    }
}

// ================================================================
// HandleHotbar()
// Handle key 1-4 untuk switch hotbar slot.
// Slot 1-2 = weapon, Slot 3-4 = potion.
// Log ke console saat switch slot.
// ================================================================
void Player::HandleHotbar(void)
{
    if (IsKeyPressed(KEY_ONE))
    {
        SelectedHotbarSlot = 0;
        TraceLog(LOG_INFO, "Player: Selected slot 1 — %s [%s]",
                 Hotbar[0].name, Hotbar[0].type == SLOT_WEAPON ? "WEAPON" : "POTION");
    }
    if (IsKeyPressed(KEY_TWO))
    {
        SelectedHotbarSlot = 1;
        TraceLog(LOG_INFO, "Player: Selected slot 2 — %s [%s]",
                 Hotbar[1].name, Hotbar[1].type == SLOT_WEAPON ? "WEAPON" : "POTION");
    }
    if (IsKeyPressed(KEY_THREE))
    {
        SelectedHotbarSlot = 2;
        TraceLog(LOG_INFO, "Player: Selected slot 3 — %s [%s]",
                 Hotbar[2].name, Hotbar[2].type == SLOT_WEAPON ? "WEAPON" : "POTION");
    }
    if (IsKeyPressed(KEY_FOUR))
    {
        SelectedHotbarSlot = 3;
        TraceLog(LOG_INFO, "Player: Selected slot 4 — %s [%s]",
                 Hotbar[3].name, Hotbar[3].type == SLOT_WEAPON ? "WEAPON" : "POTION");
    }
}

// ================================================================
// HandleActionKey()
// Handle Space — context-sensitive action.
//
// Behavior tergantung konteks:
// - Kalau inventory terbuka → equip/unequip item
// - Kalau slot weapon (1/2) → attack
// - Kalau slot potion (3/4) → minum potion
// ================================================================
void Player::HandleActionKey(void)
{
    if (!IsKeyPressed(KEY_SPACE))
        return;

    // Context: Inventory terbuka → equip/unequip
    if (bInventoryOpen)
    {
        TraceLog(LOG_INFO, "Player: [SPACE] Equip/Unequip item in inventory");
        // TODO: implement equip/unequip logic saat inventory system sudah ada
        return;
    }

    // Context: Slot weapon dipilih → attack
    HotbarSlot &currentSlot = Hotbar[SelectedHotbarSlot];
    if (currentSlot.type == SLOT_WEAPON)
    {
        State = PLAYER_ATTACKING;
        ActionTimer = ActionDuration;
        TraceLog(LOG_INFO, "Player: [SPACE] ATTACK with %s!", currentSlot.name);
        // TODO: implement damage dealing, hitbox, dsb
        return;
    }

    // Context: Slot potion dipilih → minum potion
    if (currentSlot.type == SLOT_POTION)
    {
        State = PLAYER_DRINKING_POTION;
        ActionTimer = ActionDuration;
        TraceLog(LOG_INFO, "Player: [SPACE] DRINK %s!", currentSlot.name);
        // TODO: implement healing/mana restore logic
        return;
    }
}

// ================================================================
// HandleInteract()
// Handle E — interact dengan object terdekat (NPC, chest, door, dll)
// ================================================================
void Player::HandleInteract(void)
{
    if (!IsKeyPressed(KEY_E))
        return;

    State = PLAYER_INTERACTING;
    TraceLog(LOG_INFO, "Player: [E] Interact at position (%.1f, %.1f)", Position.x, Position.y);

    // TODO: cek apakah ada interactable object di sekitar player
    // gunakan collision check ke object layer dengan type "interactable"

    // balik ke idle setelah interact (instant untuk sekarang)
    State = PLAYER_IDLE;
}

// ================================================================
// HandleInventory()
// Handle I — toggle inventory overlay
// Kalau inventory dibuka, map otomatis ditutup.
// ================================================================
void Player::HandleInventory(void)
{
    if (!IsKeyPressed(KEY_I))
        return;

    bInventoryOpen = !bInventoryOpen;

    if (bInventoryOpen)
    {
        bMapOpen = false; // tutup map kalau inventory dibuka
        TraceLog(LOG_INFO, "Player: [I] Inventory OPENED");
    }
    else
    {
        TraceLog(LOG_INFO, "Player: [I] Inventory CLOSED");
    }
}

// ================================================================
// HandleMap()
// Handle M — toggle map overlay
// Kalau map dibuka, inventory otomatis ditutup.
// ================================================================
void Player::HandleMap(void)
{
    if (!IsKeyPressed(KEY_M))
        return;

    bMapOpen = !bMapOpen;

    if (bMapOpen)
    {
        bInventoryOpen = false; // tutup inventory kalau map dibuka
        TraceLog(LOG_INFO, "Player: [M] Map OPENED");
    }
    else
    {
        TraceLog(LOG_INFO, "Player: [M] Map CLOSED");
    }
}

// ================================================================
// HandleTestDeath()
// Handle K — uji coba player mati.
// Set state ke DEAD dan bIsAlive ke false.
// ================================================================
void Player::HandleTestDeath(void)
{
    if (!IsKeyPressed(KEY_K))
        return;

    if (!bIsAlive)
        return; // sudah mati

    bIsAlive = false;
    State = PLAYER_DEAD;
    bInventoryOpen = false;
    bMapOpen = false;
    TraceLog(LOG_INFO, "Player: [K] TEST DEATH — Player is now DEAD!");
}

// ================================================================
// HandleTestRevive()
// Handle R — uji coba player hidup kembali.
// Set state ke IDLE dan bIsAlive ke true.
// ================================================================
void Player::HandleTestRevive(void)
{
    if (!IsKeyPressed(KEY_R))
        return;

    if (bIsAlive)
        return; // sudah hidup

    bIsAlive = true;
    State = PLAYER_IDLE;
    TraceLog(LOG_INFO, "Player: [R] TEST REVIVE — Player is now ALIVE!");
}

// ================================================================
// Update()
// Apply movement berdasarkan velocity yang sudah di-set HandleMovement().
// Cek collision sebelum apply posisi baru.
// Hanya jalan kalau player hidup dan tidak dalam UI mode.
// ================================================================
void Player::Update(void)
{
    // jangan gerak kalau mati, inventory/map terbuka, atau sedang aksi
    if (!bIsAlive || bInventoryOpen || bMapOpen)
        return;
    if (State == PLAYER_ATTACKING || State == PLAYER_DRINKING_POTION)
        return;

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
// Termasuk render HUD dan overlay UI.
// ================================================================
void Player::Render(void)
{
    // render sprite player (bahkan kalau mati, bisa ganti sprite nanti)
    if (bIsAlive)
    {
        RenderTilePNG(Position.x, Position.y, TILE_PLAYER_NEW, 0.0f, TEXTURE_KNIGHT);
    }
    else
    {
        // render player mati — untuk sekarang pakai tint merah
        // TODO: ganti ke death sprite/animation kalau sudah ada
        RenderTilePNG(Position.x, Position.y, TILE_PLAYER_NEW, 0.0f, TEXTURE_KNIGHT);
    }
}

// ================================================================
// RenderHUD()
// Render overlay HUD di screen space (di luar BeginMode2D).
// Nampilin hotbar, state indicator, dan overlay UI.
// ================================================================
void Player::RenderHUD(void)
{
    // ---- Hotbar Bar ----
    // posisi di tengah-bawah layar
    const int slotSize = 56;
    const int slotGap = 6;
    const int totalWidth = 4 * slotSize + 3 * slotGap;
    const int startX = (GameScreenWidth - totalWidth) / 2;
    const int startY = GameScreenHeight - slotSize - 16;

    for (int i = 0; i < 4; i++)
    {
        int x = startX + i * (slotSize + slotGap);
        bool isSelected = (i == SelectedHotbarSlot);

        // background slot
        DrawRectangle(x, startY, slotSize, slotSize, Fade(BLACK, 0.75f));

        // border — highlight kalau selected
        if (isSelected)
        {
            DrawRectangleLinesEx({(float)x, (float)startY, (float)slotSize, (float)slotSize}, 3.0f, GOLD);
        }
        else
        {
            DrawRectangleLines(x, startY, slotSize, slotSize, Fade(WHITE, 0.4f));
        }

        // nomor slot
        DrawText(TextFormat("%d", i + 1), x + 4, startY + 2, 14, isSelected ? GOLD : GRAY);

        // tipe icon (W = weapon, P = potion)
        Color typeColor = Hotbar[i].type == SLOT_WEAPON ? SKYBLUE : LIME;
        const char *typeLabel = Hotbar[i].type == SLOT_WEAPON ? "W" : "P";
        DrawText(typeLabel, x + slotSize / 2 - 5, startY + slotSize / 2 - 10, 20, typeColor);

        // nama item di bawah slot
        int nameW = MeasureText(Hotbar[i].name, 10);
        DrawText(Hotbar[i].name, x + (slotSize - nameW) / 2, startY + slotSize + 2, 10,
                 isSelected ? WHITE : Fade(WHITE, 0.5f));
    }

    // ---- Overlay UI ----
    if (bInventoryOpen)
        RenderInventoryUI();

    if (bMapOpen)
        RenderMapUI();

    if (!bIsAlive)
        RenderDeathOverlay();
}

// ================================================================
// RenderInventoryUI()
// Render overlay uji coba inventory.
// Placeholder — nanti diganti UI inventory yang proper.
// ================================================================
void Player::RenderInventoryUI(void)
{
    const int panelW = 400;
    const int panelH = 350;
    const int panelX = (GameScreenWidth - panelW) / 2;
    const int panelY = (GameScreenHeight - panelH) / 2;

    // background panel
    DrawRectangle(panelX, panelY, panelW, panelH, Fade(BLACK, 0.85f));
    DrawRectangleLinesEx({(float)panelX, (float)panelY, (float)panelW, (float)panelH}, 2.0f, GOLD);

    // title
    const char *title = "INVENTORY";
    int titleW = MeasureText(title, 24);
    DrawText(title, panelX + (panelW - titleW) / 2, panelY + 12, 24, GOLD);

    // separator line
    DrawLine(panelX + 10, panelY + 42, panelX + panelW - 10, panelY + 42, Fade(GOLD, 0.5f));

    // placeholder items
    const char *items[] = {"Sword", "Bow", "Health Potion x3", "Mana Potion x2", "Shield", "Key"};
    int itemCount = 6;

    for (int i = 0; i < itemCount; i++)
    {
        int itemY = panelY + 55 + i * 40;

        // item background
        DrawRectangle(panelX + 15, itemY, panelW - 30, 32, Fade(WHITE, 0.05f));
        DrawRectangleLines(panelX + 15, itemY, panelW - 30, 32, Fade(WHITE, 0.15f));

        // item name
        DrawText(items[i], panelX + 25, itemY + 8, 16, WHITE);

        // equip hint di item pertama
        if (i == 0)
            DrawText("[SPACE] Equip", panelX + panelW - 130, itemY + 8, 14, YELLOW);
    }

    // footer hint
    DrawText("[I] Close    [SPACE] Equip/Unequip", panelX + 15, panelY + panelH - 28, 14, GRAY);
}

// ================================================================
// RenderMapUI()
// Render overlay uji coba map.
// Placeholder — nanti diganti UI map yang proper.
// ================================================================
void Player::RenderMapUI(void)
{
    const int panelW = 500;
    const int panelH = 400;
    const int panelX = (GameScreenWidth - panelW) / 2;
    const int panelY = (GameScreenHeight - panelH) / 2;

    // background panel
    DrawRectangle(panelX, panelY, panelW, panelH, Fade(BLACK, 0.85f));
    DrawRectangleLinesEx({(float)panelX, (float)panelY, (float)panelW, (float)panelH}, 2.0f, SKYBLUE);

    // title
    const char *title = "DUNGEON MAP";
    int titleW = MeasureText(title, 24);
    DrawText(title, panelX + (panelW - titleW) / 2, panelY + 12, 24, SKYBLUE);

    // separator line
    DrawLine(panelX + 10, panelY + 42, panelX + panelW - 10, panelY + 42, Fade(SKYBLUE, 0.5f));

    // placeholder map grid
    if (tilesonMap != nullptr)
    {
        int gridW = panelW - 40;
        int gridH = panelH - 80;
        int gridX = panelX + 20;
        int gridY = panelY + 52;

        float cellW = (float)gridW / tilesonMap->width;
        float cellH = (float)gridH / tilesonMap->height;

        // render mini grid
        for (int y = 0; y < tilesonMap->height; y++)
        {
            for (int x = 0; x < tilesonMap->width; x++)
            {
                float cx = gridX + x * cellW;
                float cy = gridY + y * cellH;
                DrawRectangle((int)cx, (int)cy, (int)cellW - 1, (int)cellH - 1, Fade(DARKBLUE, 0.6f));
            }
        }

        // render player position dot
        float playerTileX = Position.x / TILE_SIZE;
        float playerTileY = Position.y / TILE_SIZE;
        float dotX = gridX + playerTileX * cellW;
        float dotY = gridY + playerTileY * cellH;
        DrawCircle((int)dotX, (int)dotY, 4.0f, GOLD);
    }
    else
    {
        DrawText("Map data not loaded", panelX + 20, panelY + 80, 18, RED);
    }

    // footer hint
    DrawText("[M] Close", panelX + 15, panelY + panelH - 28, 14, GRAY);
}

// ================================================================
// RenderDeathOverlay()
// Render overlay death screen.
// Menampilkan pesan dan hint untuk revive (R).
// ================================================================
void Player::RenderDeathOverlay(void)
{
    // full screen dark overlay
    DrawRectangle(0, 0, GameScreenWidth, GameScreenHeight, Fade(BLACK, 0.7f));

    // "YOU DIED" text
    const char *deadText = "YOU DIED";
    int textW = MeasureText(deadText, 48);
    DrawText(deadText, (GameScreenWidth - textW) / 2, GameScreenHeight / 2 - 50, 48, MAROON);

    // hint text
    const char *hintText = "Press [R] to Revive (Test)";
    int hintW = MeasureText(hintText, 20);
    DrawText(hintText, (GameScreenWidth - hintW) / 2, GameScreenHeight / 2 + 20, 20, Fade(WHITE, 0.6f));
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
// Urutan: HandleInput() → Update() → PlayerCamera()
// ================================================================
void Player::Tick(void)
{
    HandleInput();
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