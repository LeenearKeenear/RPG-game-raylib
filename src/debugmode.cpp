/**
 * @file debugmode.cpp
 * @brief Implementasi dari Debug System Module
 *
 * File ini berisi implementasi class Debug untuk:
 * - Toggle debug mode
 * - Render panel debug
 * - Render overlay debug di world space
 * - Menampilkan info runtime player, map, camera, dan collision
 */

#include "../include/debug.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/screen.h"
#include "../include/map.h"
#include "../include/mapLogic.h"
#include "../include/animation.h"
#include "../include/player.h"

/*==============================================================================
 * Global Variables
 *==============================================================================*/

/** Global instance debug yang dipakai lintas file */
Debug DebugInstance;

/** Flag untuk menentukan apakah debug mode sedang aktif */
bool isDebugMode = false;

/*==============================================================================
 * Private Helper Methods
 *==============================================================================*/

/**
 * @brief Hitung bounds panel berdasarkan urutan tampil
 *
 * Panel disusun dalam layout grid dua kolom dari pojok kiri atas layar.
 *
 * @param index Urutan panel yang akan digambar
 * @param panelWidth Lebar panel
 * @param panelHeight Tinggi panel
 * @return Rectangle posisi dan ukuran panel
 */
Rectangle Debug::GetPanelBounds(int index, float panelWidth, float panelHeight) const
{
    // Konfigurasi grid panel debug
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

/**
 * @brief Bangun daftar panel debug yang aktif
 *
 * Urutan panel dalam vector akan dipakai sebagai urutan layout di layar.
 *
 * @return Vector berisi panel yang akan dirender
 */
std::vector<Debug::DebugPanelEntry> Debug::BuildActivePanels(void) const
{
    std::vector<DebugPanelEntry> panels;

    // Daftar panel aktif yang akan dirender
    // panels.push_back({"Map", &Debug::DrawMapPanel, true});
    // panels.push_back({"Camera", &Debug::DrawCameraPanel, true});
    panels.push_back({"Player", &Debug::DrawPlayerPanel, true});
    // panels.push_back({"Zoom", &Debug::DrawZoomPanel, true});
    panels.push_back({"Frustum", &Debug::DrawFrustumPanel, true});
    panels.push_back({"Collision", &Debug::DrawCollisionPanel, true});

    return panels;
}

/**
 * @brief Gambar frame dasar untuk satu panel debug
 *
 * Panel digambar dengan background hitam semi-transparan,
 * border berwarna, dan judul di bagian atas.
 *
 * @param bounds Area panel
 * @param title Judul panel
 * @param borderColor Warna border dan judul
 */
void Debug::DrawPanelFrame(Rectangle bounds, const char *title, Color borderColor) const
{
    DrawRectangle((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height, Fade(BLACK, 0.7f));
    DrawRectangleLines((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height, borderColor);
    DrawText(title, (int)bounds.x + 10, (int)bounds.y + 5, 18, borderColor);
}

/**
 * @brief Gambar overlay collision untuk layer tertentu
 *
 * Object rectangle digambar sebagai outline rectangle,
 * sedangkan object polygon digambar per edge dan titik sudut.
 *
 * @param layerName Nama layer collision
 * @param rectColor Warna rectangle collision
 * @param polygonColor Warna edge polygon
 * @param pointColor Warna titik polygon
 */
void Debug::DrawCollisionOverlay(const std::string &layerName, Color rectColor, Color polygonColor, Color pointColor)
{
    const std::vector<MapObject *> &objs = TilesonGetObjectsByLayerName(layerName.c_str());

    for (auto *obj : objs)
    {
        if (obj->hasPolygon)
        {
            // Gambar setiap edge dan titik polygon
            int pointCount = (int)obj->polygonPoints.size();
            if (pointCount >= 2)
            {
                for (int i = 0; i < pointCount; i++)
                {
                    int nextIndex = (i + 1) % pointCount;
                    DrawLineEx(obj->polygonPoints[i], obj->polygonPoints[nextIndex], 2.0f, polygonColor);
                    DrawCircleV(obj->polygonPoints[i], 3.0f, pointColor);
                }
            }
        }
        else
        {
            // Gambar rectangle collision
            DrawRectangleLinesEx(obj->bounds, 2.0f, rectColor);
        }
    }
}

/**
 * @brief Gambar overlay raycast interaksi player
 *
 * Ray diambil dari tengah hitbox player menuju arah mouse,
 * lalu titik hit terakhir ditandai jika ada object yang terkena.
 */
void Debug::DrawRaycastOverlay(void)
{
    Vector2 playerCenter = {
        PlayerInstance.GetPosition().x + PlayerInstance.GetHitboxOffsetX() + PlayerInstance.GetHitboxWidth() / 2,
        PlayerInstance.GetPosition().y + PlayerInstance.GetHitboxOffsetY() + PlayerInstance.GetHitboxHeight() / 2};

    Vector2 mouseWorld = GetScreenToWorld2D(GetVirtualMousePosition(gState), camera);
    Vector2 aimDir = Vector2Normalize(Vector2Subtract(mouseWorld, playerCenter));
    Vector2 rayEnd = {
        playerCenter.x + aimDir.x * PlayerInstance.GetINTERACT_RANGE(),
        playerCenter.y + aimDir.y * PlayerInstance.GetINTERACT_RANGE()};

    DrawLineEx(playerCenter, rayEnd, 2.0f, GREEN);

    // Gambar titik hit jika ray mengenai object
    if (PlayerInstance.GetLastHit().hit)
        DrawCircleV(PlayerInstance.GetLastHit().point, 4.0f, RED);
}

/*==============================================================================
 * Public Methods
 *==============================================================================*/

/**
 * @brief Toggle debug mode saat tombol TAB ditekan
 */
void Debug::Toggle(void)
{
    if (IsKeyPressed(KEY_TAB))
    {
        isDebugMode = !isDebugMode;
        TraceLog(LOG_INFO, "Debug mode: %s", isDebugMode ? "ON" : "OFF");
    }
}

/**
 * @brief Render seluruh panel debug yang aktif
 *
 * Fungsi ini hanya berjalan saat debug mode aktif.
 */
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

/*==============================================================================
 * Debug Panels
 *==============================================================================*/

/**
 * @brief Gambar panel informasi map
 *
 * Panel ini menampilkan ukuran map, jumlah layer, dan status tileset.
 *
 * @param bounds Area panel
 */
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

/**
 * @brief Gambar panel informasi camera
 *
 * @param bounds Area panel
 */
void Debug::DrawCameraPanel(Rectangle bounds)
{
    DrawPanelFrame(bounds, "[ CAMERA DEBUG ]", SKYBLUE);
    DrawText(TextFormat("Target  : (%.1f, %.1f)", camera.target.x, camera.target.y),
             (int)bounds.x + 10, (int)bounds.y + 27, 16, WHITE);
    DrawText(TextFormat("Zoom    : %.2f", camera.zoom),
             (int)bounds.x + 10, (int)bounds.y + 47, 16, WHITE);
}

/**
 * @brief Gambar panel informasi player
 *
 * Panel ini menampilkan posisi, speed, ukuran hitbox, dan offset hitbox.
 *
 * @param bounds Area panel
 */
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

/**
 * @brief Gambar panel zoom debug dan handle input zoom
 *
 * Zoom hanya bisa diubah saat debug mode aktif.
 *
 * @param bounds Area panel
 */
void Debug::DrawZoomPanel(Rectangle bounds)
{
    const float MAX_ZOOM = 3.5f;
    const float MIN_ZOOM = 0.85f;
    const float ZOOM_INCREMENT = 0.25f;

    // Handle zoom dengan scroll mouse
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

/**
 * @brief Gambar panel informasi frustum culling
 *
 * Panel ini menampilkan jumlah tile yang dirender dan range tile visible.
 *
 * @param bounds Area panel
 */
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

/**
 * @brief Gambar panel informasi collision dan boundary
 *
 * Panel ini menampilkan jumlah object collision rectangle dan polygon
 * yang terdeteksi dari layer collision aktif.
 *
 * @param bounds Area panel
 */
void Debug::DrawCollisionPanel(Rectangle bounds)
{
    if (tilesonMap == nullptr)
        return;

    int collisionRectCount = 0;
    int collisionPolygonCount = 0;
    int mapBoundPolygonCount = 0;

    // Hitung object collision berdasarkan layer name
    const std::vector<MapObject *> &collisionObjs = TilesonGetObjectsByLayerName(COLLISION_LAYER_NAME);
    for (auto *obj : collisionObjs)
    {
        if (obj->hasPolygon)
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

/*==============================================================================
 * World Overlay
 *==============================================================================*/

/**
 * @brief Render overlay debug langsung di world space
 *
 * Overlay ini menampilkan hitbox player, object collision,
 * raycast interaksi, dan batas map aktif.
 */
void Debug::DrawWorldOverlay(void)
{
    if (!isDebugMode)
        return;

    if (tilesonMap == nullptr)
        return;

    // Hitbox player
    Vector2 playerPos = PlayerInstance.GetPosition();

    Rectangle playerHitbox = {
        playerPos.x + PlayerInstance.GetHitboxOffsetX(),
        playerPos.y + PlayerInstance.GetHitboxOffsetY(),
        PlayerInstance.GetHitboxWidth(),
        PlayerInstance.GetHitboxHeight()};

    DrawRectangleLinesEx(playerHitbox, 2.0f, LIME);

    // Titik sudut hitbox player
    DrawCircleV({playerHitbox.x, playerHitbox.y}, 2.5f, GREEN);
    DrawCircleV({playerHitbox.x + playerHitbox.width, playerHitbox.y}, 2.5f, GREEN);
    DrawCircleV({playerHitbox.x, playerHitbox.y + playerHitbox.height}, 2.5f, GREEN);
    DrawCircleV({playerHitbox.x + playerHitbox.width, playerHitbox.y + playerHitbox.height}, 2.5f, GREEN);

    // Overlay collision object
    DrawCollisionOverlay(COLLISION_LAYER_NAME, RED, GOLD, GOLD);
    DrawCollisionOverlay(OBJECT_LAYER_NAME, BLUE, BLUE, BLUE);
    DrawRaycastOverlay();

    // Batas luar map
    Rectangle mapBounds = {
        0.0f,
        0.0f,
        (float)tilesonMap->width * TILE_SIZE,
        (float)tilesonMap->height * TILE_SIZE};

    DrawRectangleLinesEx(mapBounds, 2.0f, SKYBLUE);

    // Label kecil untuk hitbox
    DrawText("Hitbox", (int)playerHitbox.x, (int)playerHitbox.y - 14, 14, LIME);
}
