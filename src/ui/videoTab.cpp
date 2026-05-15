/**
 * @file videoTab.cpp
 * @brief Implementasi Video Settings Tab
 * 
 * Handle rendering dan input untuk pengaturan video.
 */

#include "videoTab.h"
#include "screen.h"
#include "../lib/raylib/include/raylib.h"

void DrawVideoTab(
    buttonTxt& fullscreenButton,
    buttonTxt& fpsButton,
    Vector2 mousePosition,
    int startX,
    int startY)
{
    int contentStartY = startY + 100;
    const int fontSize = 24;
    int labelX = startX + 40;

    DrawText("Fullscreen", labelX, contentStartY + 15, fontSize, WHITE);
    DrawText("Show FPS", labelX, contentStartY + 75, fontSize, WHITE);

    fullscreenButton.Draw(mousePosition);
    fpsButton.Draw(mousePosition);
}

bool UpdateVideoTab(
    buttonTxt& fullscreenButton,
    buttonTxt& fpsButton,
    void* stateVoid,
    Vector2 mousePosition,
    bool mouseClicked)
{
    auto* state = static_cast<GameState*>(stateVoid);

    if (fullscreenButton.isClicked(mousePosition, mouseClicked)) {
        if (IsWindowFullscreen()) {
            ToggleFullscreenMode();
        } else {
            Rectangle monitorRes = GetMonitorResolution();
            SetWindowSize(
                static_cast<int>(monitorRes.width),
                static_cast<int>(monitorRes.height));
            ToggleFullscreenMode();
        }
        return true;
    }

    if (fpsButton.isClicked(mousePosition, mouseClicked)) {
        state->showFPS = !state->showFPS;
        return true;
    }

    return false;
}
