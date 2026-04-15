#include "../include/debug.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/screen.h"
#include "../include/map.h"
#include "../include/player.h"

// ================================================================
// Global
// ================================================================

// global instance debug — diakses file lain via extern
Debug DebugInstance;
bool isDebugMode = false;

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

    DrawMapPanel();
    DrawCameraPanel();
    DrawPlayerPanel();
    DrawZoomPanel();
}

// ================================================================
// DrawMapPanel()
// Panel info map: ukuran dalam tile, jumlah layer, status tileset.
// Di-skip kalau tilesonMap belum ke-load.
// ================================================================
void Debug::DrawMapPanel(void)
{
    if (tilesonMap == nullptr)
        return;

    DrawRectangle(5, 5, 270, 115, Fade(BLACK, 0.7f));
    DrawRectangleLines(5, 5, 270, 115, YELLOW);
    DrawText("[ MAP DEBUG ]", 15, 10, 18, YELLOW);
    DrawText(TextFormat("Size    : %dx%d tiles", tilesonMap->width, tilesonMap->height), 15, 32, 16, WHITE);
    DrawText(TextFormat("Layers  : %d", tilesonMap->layerCount), 15, 52, 16, WHITE);
    DrawText(TextFormat("Tileset : %s", tilesonMap->tilesetTexture.id != 0 ? "Loaded" : "Not loaded"),
             15, 72, 16, tilesonMap->tilesetTexture.id != 0 ? GREEN : RED);
}

// ================================================================
// DrawCameraPanel()
// Panel info camera: posisi target dan zoom saat ini.
// ================================================================
void Debug::DrawCameraPanel(void)
{
    DrawRectangle(5, 130, 270, 70, Fade(BLACK, 0.7f));
    DrawRectangleLines(5, 130, 270, 70, SKYBLUE);
    DrawText("[ CAMERA DEBUG ]", 15, 135, 18, SKYBLUE);
    DrawText(TextFormat("Target  : (%.1f, %.1f)", camera.target.x, camera.target.y), 15, 155, 16, WHITE);
    DrawText(TextFormat("Zoom    : %.2f", camera.zoom), 15, 175, 16, WHITE);
}

// ================================================================
// DrawPlayerPanel()
// Panel info player: posisi dalam pixel (float) dan speed.
// ================================================================
void Debug::DrawPlayerPanel(void)
{
    // state name mapping
    const char *stateNames[] = {"IDLE", "MOVING", "ATTACKING", "DRINKING_POTION", "INTERACTING", "DEAD"};
    PlayerState state = PlayerInstance.GetState();
    const char *stateName = (state >= 0 && state <= 5) ? stateNames[state] : "UNKNOWN";

    DrawRectangle(5, 210, 270, 150, Fade(BLACK, 0.7f));
    DrawRectangleLines(5, 210, 270, 150, GREEN);
    DrawText("[ PLAYER DEBUG ]", 15, 215, 18, GREEN);
    DrawText(TextFormat("Position: (%.1f, %.1f)", PlayerInstance.GetPosition().x, PlayerInstance.GetPosition().y), 15, 235, 16, WHITE);
    DrawText(TextFormat("Speed   : %.1f", PlayerInstance.GetSpeed()), 15, 255, 16, WHITE);
    DrawText(TextFormat("State   : %s", stateName), 15, 275, 16,
             state == PLAYER_DEAD ? RED : (state == PLAYER_ATTACKING ? ORANGE : WHITE));
    DrawText(TextFormat("Alive   : %s", PlayerInstance.IsAlive() ? "YES" : "NO"), 15, 295, 16,
             PlayerInstance.IsAlive() ? GREEN : RED);
    DrawText(TextFormat("Slot    : %d (%s)", PlayerInstance.GetSelectedSlot() + 1,
             PlayerInstance.GetHotbarSlot(PlayerInstance.GetSelectedSlot()).name), 15, 315, 16, GOLD);
    DrawText(TextFormat("Inv/Map : %s / %s",
             PlayerInstance.IsInventoryOpen() ? "OPEN" : "closed",
             PlayerInstance.IsMapOpen() ? "OPEN" : "closed"), 15, 335, 16, SKYBLUE);
}

// ================================================================
// DrawZoomPanel()
// Panel zoom debug: nampilin zoom saat ini + handle input scroll.
// Zoom hanya bisa diubah kalau isDebugMode true.
// ================================================================
void Debug::DrawZoomPanel(void)
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

    DrawRectangle(5, 370, 270, 70, Fade(BLACK, 0.7f));
    DrawRectangleLines(5, 370, 270, 70, ORANGE);
    DrawText("[ ZOOM DEBUG ]", 15, 375, 18, ORANGE);
    DrawText(TextFormat("Zoom    : %.2f", camera.zoom), 15, 395, 16, WHITE);
    DrawText("[Scroll] Zoom In/Out", 15, 415, 16, YELLOW);
}

// ================================================================
// DrawFrustumPanel()
// Panel info frustum culling: jumlah tile yang di-render vs total map,
// serta jangkauan index tile (min/max) yang terlihat.
// ================================================================
// void Debug::DrawFrustumPanel(void)
// {
//     if (tilesonMap == nullptr)
//         return;

//     int totalMapTiles = tilesonMap->width * tilesonMap->height * tilesonMap->layerCount;

//     DrawRectangle(5, 450, 270, 95, Fade(BLACK, 0.7f));
//     DrawRectangleLines(5, 450, 270, 95, VIOLET);
//     DrawText("[ FRUSTUM DEBUG ]", 15, 455, 18, VIOLET);
//     DrawText(TextFormat("Tiles Drawn : %d", lastTilesRendered), 15, 475, 16, WHITE);
//     DrawText(TextFormat("Total Map   : %d", totalMapTiles), 15, 495, 16, GRAY);
//     DrawText(TextFormat("Range X: %d-%d", currentVisibleRange.minX, currentVisibleRange.maxX), 15, 515, 16, WHITE);
//     DrawText(TextFormat("Range Y: %d-%d", currentVisibleRange.minY, currentVisibleRange.maxY), 15, 535, 16, WHITE);
// }

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