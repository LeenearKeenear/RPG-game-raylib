#include "../include/debug.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/screen.h"
#include "../include/map.h"
#include "../include/animation.h"
#include "../include/player.h"

// ================================================================
// Global
// ================================================================

// global instance debug — diakses file lain via extern
Debug DebugInstance;
bool isDebugMode = false;

Rectangle Debug::GetPanelBounds(int index, float panelWidth, float panelHeight) const
{
    // konfigurasi grid debug panel
    const int ColumnCount = 2;
    const float StartX = 5.0f;
    const float StartY = 5.0f;
    const float GapX = 10.0f;
    const float GapY = 10.0f;

    int column = index % ColumnCount;
    int row = index / ColumnCount;

    Rectangle bounds = {
        StartX + column * (panelWidth + GapX),
        StartY + row * (panelHeight + GapY),
        panelWidth,
        panelHeight};

    return bounds;
}

std::vector<Debug::DebugPanelEntry> Debug::BuildActivePanels(void) const
{
    std::vector<DebugPanelEntry> panels;

    // daftar panel aktif
    // urutan di vector = urutan layout di grid
    panels.push_back({"Map", &Debug::DrawMapPanel, true});
    // panels.push_back({"Camera", &Debug::DrawCameraPanel, true});
    // panels.push_back({"Player", &Debug::DrawPlayerPanel, true});
    panels.push_back({"Zoom", &Debug::DrawZoomPanel, true});
    // panels.push_back({"Frustum", &Debug::DrawFrustumPanel, true});
    panels.push_back({"Collision", &Debug::DrawCollisionPanel, true});

    return panels;
}

void Debug::DrawPanelFrame(Rectangle bounds, const char *title, Color borderColor) const
{
    DrawRectangle((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height, Fade(BLACK, 0.7f));
    DrawRectangleLines((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height, borderColor);
    DrawText(title, (int)bounds.x + 10, (int)bounds.y + 5, 18, borderColor);
}

void Debug::DrawCollisionOverlay(const std::string& layerName, Color rectColor, Color polygonColor, Color pointColor)
{
    std::vector<MapObject> objs = TilesonGetObjectsByLayerName(layerName.c_str());

    for (auto& obj : objs)
    {
        if (obj.hasPolygon)
        {
            int pointCount = (int)obj.polygonPoints.size();
            if (pointCount >= 2)
            {
                for (int i = 0; i < pointCount; i++)
                {
                    int nextIndex = (i + 1) % pointCount;
                    DrawLineEx(obj.polygonPoints[i], obj.polygonPoints[nextIndex], 2.0f, polygonColor);
                    DrawCircleV(obj.polygonPoints[i], 3.0f, pointColor);
                }
            }
        }
        else
        {
            DrawRectangleLinesEx(obj.bounds, 2.0f, rectColor);
        }
    }
}

// ================================================================
// Toggle()
// Handle input TAB buat nyalain/matiin debug mode.
// Log ke console tiap kali state berubah.
// ================================================================
void Debug::Toggle(void)
{
    if (IsKeyPressed(KEY_TAB))
    {
        isDebugMode = !isDebugMode;
        TraceLog(LOG_INFO, "Debug mode: %s", isDebugMode ? "ON" : "OFF");
    }
}

// ================================================================
// Draw()
// Wrapper semua panel debug.
// Semua panel hanya dirender kalau isDebugMode true.
// ================================================================
void Debug::Draw(void)
{
    if (!isDebugMode)
        return;

    std::vector<DebugPanelEntry> panels = BuildActivePanels();

    const float DefaultPanelWidth = 320.0f;
    const float DefaultPanelHeight = 110.0f;

    int visibleIndex = 0;
    for (auto &panel : panels)
    {
        if (!panel.enabled)
            continue;

        Rectangle bounds = GetPanelBounds(visibleIndex, DefaultPanelWidth, DefaultPanelHeight);
        (this->*panel.drawFn)(bounds);
        visibleIndex++;
    }
}

// ================================================================
// DrawMapPanel()
// Panel info map: ukuran dalam tile, jumlah layer, status tileset.
// Di-skip kalau tilesonMap belum ke-load.
// ================================================================
void Debug::DrawMapPanel(Rectangle bounds)
{
    if (tilesonMap == nullptr)
        return;

    DrawPanelFrame(bounds, "[ MAP DEBUG ]", YELLOW);
    DrawText(TextFormat("Size    : %dx%d tiles", tilesonMap->width, tilesonMap->height),
             (int)bounds.x + 10, (int)bounds.y + 27, 16, WHITE);
    DrawText(TextFormat("Layers  : %d", tilesonMap->layerCount),
             (int)bounds.x + 10, (int)bounds.y + 47, 16, WHITE);
    DrawText(TextFormat("Tileset : %s", !tilesonMap->tilesets.empty() ? "Loaded" : "Not loaded"),
             (int)bounds.x + 10, (int)bounds.y + 67, 16, !tilesonMap->tilesets.empty() ? GREEN : RED);
}

// ================================================================
// DrawCameraPanel()
// Panel info camera: posisi target dan zoom saat ini.
// ================================================================
void Debug::DrawCameraPanel(Rectangle bounds)
{
    DrawPanelFrame(bounds, "[ CAMERA DEBUG ]", SKYBLUE);
    DrawText(TextFormat("Target  : (%.1f, %.1f)", camera.target.x, camera.target.y),
             (int)bounds.x + 10, (int)bounds.y + 27, 16, WHITE);
    DrawText(TextFormat("Zoom    : %.2f", camera.zoom),
             (int)bounds.x + 10, (int)bounds.y + 47, 16, WHITE);
}

// ================================================================
// DrawPlayerPanel()
// Panel info player: posisi dalam pixel (float) dan speed.
// ================================================================
void Debug::DrawPlayerPanel(Rectangle bounds)
{
    DrawPanelFrame(bounds, "[ PLAYER DEBUG ]", GREEN);
    DrawText(TextFormat("Position   : (%.1f, %.1f)", PlayerInstance.GetPosition().x, PlayerInstance.GetPosition().y),
             (int)bounds.x + 10, (int)bounds.y + 27, 16, WHITE);
    DrawText(TextFormat("Speed      : %.1f", PlayerInstance.GetSpeed()),
             (int)bounds.x + 10, (int)bounds.y + 47, 16, WHITE);
    DrawText(TextFormat("Hitbox Size: %.1f x %.1f", PlayerInstance.GetHitboxWidth(), PlayerInstance.GetHitboxHeight()),
             (int)bounds.x + 10, (int)bounds.y + 67, 16, YELLOW);
    DrawText(TextFormat("Hitbox Off : (%.1f, %.1f)", PlayerInstance.GetHitboxOffsetX(), PlayerInstance.GetHitboxOffsetY()),
             (int)bounds.x + 10, (int)bounds.y + 87, 16, YELLOW);
}

// ================================================================
// DrawZoomPanel()
// Panel zoom debug: nampilin zoom saat ini + handle input scroll.
// Zoom hanya bisa diubah kalau isDebugMode true.
// ================================================================
void Debug::DrawZoomPanel(Rectangle bounds)
{
    const float MAX_ZOOM = 3.5f;
    const float MIN_ZOOM = 0.85f;
    const float ZOOM_INCREMENT = 0.25f;

    // handle zoom input via scroll
    float MouseWheel = GetMouseWheelMove();
    if (MouseWheel != 0)
    {
        camera.zoom += MouseWheel * ZOOM_INCREMENT;
        if (camera.zoom > MAX_ZOOM)
            camera.zoom = MAX_ZOOM;
        if (camera.zoom < MIN_ZOOM)
            camera.zoom = MIN_ZOOM;
    }

    DrawPanelFrame(bounds, "[ ZOOM DEBUG ]", ORANGE);
    DrawText(TextFormat("Zoom    : %.2f", camera.zoom),
             (int)bounds.x + 10, (int)bounds.y + 27, 16, WHITE);
    DrawText("[Scroll] Zoom In/Out",
             (int)bounds.x + 10, (int)bounds.y + 47, 16, YELLOW);
}

// ================================================================
// DrawFrustumPanel()
// Panel info frustum culling: jumlah tile yang di-render vs total map,
// serta jangkauan index tile (min/max) yang terlihat.
// ================================================================
void Debug::DrawFrustumPanel(Rectangle bounds)
{
    if (tilesonMap == nullptr)
        return;

    int totalMapTiles = tilesonMap->width * tilesonMap->height * tilesonMap->layerCount;

    DrawPanelFrame(bounds, "[ FRUSTUM DEBUG ]", VIOLET);
    DrawText(TextFormat("Tiles Drawn : %d", lastTilesRendered),
             (int)bounds.x + 10, (int)bounds.y + 27, 16, WHITE);
    DrawText(TextFormat("Total Map   : %d", totalMapTiles),
             (int)bounds.x + 10, (int)bounds.y + 47, 16, GRAY);
    DrawText(TextFormat("Range X: %d-%d", currentVisibleRange.minX, currentVisibleRange.maxX),
             (int)bounds.x + 10, (int)bounds.y + 67, 16, WHITE);
    DrawText(TextFormat("Range Y: %d-%d", currentVisibleRange.minY, currentVisibleRange.maxY),
             (int)bounds.x + 10, (int)bounds.y + 87, 16, WHITE);
}

// ================================================================
// DrawCollisionPanel()
// Panel info collision & world boundary:
// - jumlah collision rect
// - jumlah collision polygon
// - status custom world boundary
//
// Tujuannya buat validasi cepat apakah object dari layer collision
// dan map_bound sudah kebaca dengan benar pas runtime.
// ================================================================
void Debug::DrawCollisionPanel(Rectangle bounds)
{
    if (tilesonMap == nullptr)
        return;

    int collisionRectCount = 0;
    int collisionPolygonCount = 0;
    int mapBoundPolygonCount = 0;

    // hitung object collision berdasarkan layer name
    std::vector<MapObject> collisionObjs = TilesonGetObjectsByLayerName(COLLISION_LAYER_NAME);
    for (auto &obj : collisionObjs)
    {
        if (obj.hasPolygon)
            collisionPolygonCount++;
        else
            collisionRectCount++;
    }

    DrawPanelFrame(bounds, "[ COLLISION DEBUG ]", RED);
    DrawText(TextFormat("Rect Count     : %d", collisionRectCount),
             (int)bounds.x + 10, (int)bounds.y + 27, 16, WHITE);
    DrawText(TextFormat("Polygon Count  : %d", collisionPolygonCount),
             (int)bounds.x + 10, (int)bounds.y + 47, 16, WHITE);
    DrawText(TextFormat("Map Bound Poly : %d", mapBoundPolygonCount),
             (int)bounds.x + 10, (int)bounds.y + 67, 16, WHITE);
    DrawText(TextFormat("Boundary Mode  : %s", mapBoundPolygonCount > 0 ? "Custom Polygon" : "Default Rectangle"),
             (int)bounds.x + 10, (int)bounds.y + 87, 16, mapBoundPolygonCount > 0 ? GREEN : YELLOW);
}

void Debug::DrawWorldOverlay(void)
{
    if (!isDebugMode)
        return;

    if (tilesonMap == nullptr)
        return;

    // ============================================================
    // hitbox player
    // ambil dari data player biar kalau size hitbox diubah di Player,
    // overlay debug otomatis ikut update.
    // ============================================================
    Vector2 playerPos = PlayerInstance.GetPosition();

    Rectangle playerHitbox = {
        playerPos.x + PlayerInstance.GetHitboxOffsetX(),
        playerPos.y + PlayerInstance.GetHitboxOffsetY(),
        PlayerInstance.GetHitboxWidth(),
        PlayerInstance.GetHitboxHeight()};

    DrawRectangleLinesEx(playerHitbox, 2.0f, LIME);

    // titik sudut hitbox player biar gampang cek rasa collision di pojok
    DrawCircleV({playerHitbox.x, playerHitbox.y}, 2.5f, GREEN);
    DrawCircleV({playerHitbox.x + playerHitbox.width, playerHitbox.y}, 2.5f, GREEN);
    DrawCircleV({playerHitbox.x, playerHitbox.y + playerHitbox.height}, 2.5f, GREEN);
    DrawCircleV({playerHitbox.x + playerHitbox.width, playerHitbox.y + playerHitbox.height}, 2.5f, GREEN);

    // ============================================================
    // collision object dari layer obstacle
    // rectangle digambar merah, polygon digambar oranye.
    // semua data diambil langsung dari map helper berdasarkan define layer.
    // ============================================================

    DrawCollisionOverlay(COLLISION_LAYER_NAME, RED, GOLD, GOLD);
    DrawCollisionOverlay(OBJECT_LAYER_NAME, BLUE, BLUE, BLUE);

    // ============================================================
    // batas luar map rectangle
    // ini nunjukin world bound aktif yang sekarang dipakai oleh CanMove().
    // ============================================================
    Rectangle mapBounds = {
        0.0f,
        0.0f,
        (float)tilesonMap->width * TILE_SIZE,
        (float)tilesonMap->height * TILE_SIZE};

    DrawRectangleLinesEx(mapBounds, 2.0f, SKYBLUE);

    // ============================================================
    // label kecil biar kebaca pas runtime
    // ============================================================
    DrawText("Hitbox", (int)playerHitbox.x, (int)playerHitbox.y - 14, 14, LIME);
}

// ================================================================
// DebugMouse() — di-comment, sapa tau butuh nanti
// Nampilin posisi mouse di screen asli dan layar virtual.
// ================================================================
// void Debug::DebugMouse(GameState *state)
// {
//     Vector2 Mouse = GetMousePosition();
//     Vector2 virtualMouse = {0, 0};
//     virtualMouse.x = (Mouse.x - ((state->WindowScreenWidth - (GameScreenWidth * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;
//     virtualMouse.y = (Mouse.y - ((state->WindowScreenHeight - (GameScreenHeight * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;
//     virtualMouse = Vector2Clamp(virtualMouse, (Vector2){0, 0}, (Vector2){(float)GameScreenWidth, (float)GameScreenHeight});
//
//     DrawText(TextFormat("Default Mouse: [%i , %i]", (int)Mouse.x, (int)Mouse.y), 5, 240, 25, GREEN);
//     DrawText(TextFormat("Virtual Mouse: [%i , %i]", (int)virtualMouse.x, (int)virtualMouse.y), 5, 265, 25, YELLOW);
// }