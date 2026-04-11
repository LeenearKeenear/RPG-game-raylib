#include "../include/debug.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/screen.h"
#include "../include/map.h"
#include "../include/player.h"


// definisi global
Debug DebugInstance;
bool isDebugMode = false;

// ================================================================
// Toggle()
// Handle input TAB buat nyalain/matiin debug mode
// ================================================================
void Debug::Toggle()
{
    if (IsKeyPressed(KEY_TAB))
    {
        isDebugMode = !isDebugMode;
        TraceLog(LOG_INFO, "Debug mode: %s", isDebugMode ? "ON" : "OFF");
    }
}

// ================================================================
// Draw()
// Wrapper semua panel debug, hanya aktif kalau isDebugMode true
// ================================================================
void Debug::Draw()
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
// Nampilin info map: ukuran, jumlah layer, status tileset
// ================================================================
void Debug::DrawMapPanel()
{
    if (tilesonMap == nullptr)
        return;

    DrawRectangle(5, 5, 270, 115, Fade(BLACK, 0.7f));
    DrawRectangleLines(5, 5, 270, 115, YELLOW);
    DrawText("[ MAP DEBUG ]", 15, 10, 18, YELLOW);
    DrawText(TextFormat("Size    : %dx%d tiles", tilesonMap->width, tilesonMap->height), 15, 32, 16, WHITE);
    DrawText(TextFormat("Layers  : %d", tilesonMap->layerCount), 15, 52, 16, WHITE);
    DrawText(TextFormat("Tileset : %s", tilesonMap->tilesetTexture.id != 0 ? "Loaded" : "Not loaded"), 15, 72, 16, tilesonMap->tilesetTexture.id != 0 ? GREEN : RED);
}

// ================================================================
// DrawCameraPanel()
// Nampilin info camera: target position dan zoom
// ================================================================
void Debug::DrawCameraPanel()
{
    DrawRectangle(5, 130, 270, 70, Fade(BLACK, 0.7f));
    DrawRectangleLines(5, 130, 270, 70, SKYBLUE);
    DrawText("[ CAMERA DEBUG ]", 15, 135, 18, SKYBLUE);
    DrawText(TextFormat("Target  : (%.1f, %.1f)", camera.target.x, camera.target.y), 15, 155, 16, WHITE);
    DrawText(TextFormat("Zoom    : %.2f", camera.zoom), 15, 175, 16, WHITE);
}

// ================================================================
// DrawPlayerPanel()
// Nampilin info player: posisi dalam pixel (float)
// ================================================================
void Debug::DrawPlayerPanel()
{
    DrawRectangle(5, 210, 270, 70, Fade(BLACK, 0.7f));
    DrawRectangleLines(5, 210, 270, 70, GREEN);
    DrawText("[ PLAYER DEBUG ]", 15, 215, 18, GREEN);
    DrawText(TextFormat("Position: (%.1f, %.1f)", PlayerInstance.GetPosition().x, PlayerInstance.GetPosition().y), 15, 235, 16, WHITE);
    DrawText(TextFormat("Speed   : %.1f", PlayerInstance.GetSpeed()), 15, 255, 16, WHITE);
}

// ================================================================
// DrawZoomPanel()
// Nampilin info zoom debug + handle input zoom via mouse wheel
// ================================================================
void Debug::DrawZoomPanel()
{
    const float MAX_ZOOM = 3.5f;
    const float MIN_ZOOM = 0.85f;
    const float ZOOM_INCREMENT = 0.25f;

    // handle zoom input
    float MouseWheel = GetMouseWheelMove();
    if (MouseWheel != 0)
    {
        camera.zoom += MouseWheel * ZOOM_INCREMENT;
        if (camera.zoom > MAX_ZOOM)
            camera.zoom = MAX_ZOOM;
        if (camera.zoom < MIN_ZOOM)
            camera.zoom = MIN_ZOOM;
    }

    DrawRectangle(5, 290, 270, 70, Fade(BLACK, 0.7f));
    DrawRectangleLines(5, 290, 270, 70, ORANGE);
    DrawText("[ ZOOM DEBUG ]", 15, 295, 18, ORANGE);
    DrawText(TextFormat("Zoom    : %.2f", camera.zoom), 15, 315, 16, WHITE);
    DrawText("[Scroll] Zoom In/Out", 15, 335, 16, YELLOW);
}

// ================================================================
// DebugZoom()
// Fungsi debug buat ngatur zoom camera secara manual pake mouse wheel.
// Dipanggil hanya kalau debug mode aktif (isDebugMode == true).
// ================================================================
void Debug::DebugZoom(void)
{
    const float MAX_ZOOM = 3.5f;
    const float MIN_ZOOM = 0.85f;
    const float ZOOM_INCREMENT = 0.25f;

    float MouseWheel = GetMouseWheelMove();
    if (MouseWheel != 0)
    {
        camera.zoom += MouseWheel * ZOOM_INCREMENT;
        if (camera.zoom > MAX_ZOOM)
            camera.zoom = MAX_ZOOM;
        if (camera.zoom < MIN_ZOOM)
            camera.zoom = MIN_ZOOM;
    }
}

// isinya buat debug menu posisi mouse, sapa tau butuh kan
void Debug::DebugMouse(GameState *state)
{
    Vector2 Mouse = GetMousePosition();
    Vector2 virtualMouse = {0, 0};
    virtualMouse.x = (Mouse.x - ((state->WindowScreenWidth - (GameScreenWidth * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;
    virtualMouse.y = (Mouse.y - ((state->WindowScreenHeight - (GameScreenHeight * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;
    virtualMouse = Vector2Clamp(virtualMouse, (Vector2){0, 0}, (Vector2){(float)GameScreenWidth, (float)GameScreenHeight});

    DrawText(TextFormat("Default Mouse: [%i , %i]", (int)Mouse.x, (int)Mouse.y), 5, 240, 25, GREEN);
    DrawText(TextFormat("Virtual Mouse: [%i , %i]", (int)virtualMouse.x, (int)virtualMouse.y), 5, 265, 25, YELLOW);
}