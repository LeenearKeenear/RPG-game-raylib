/**
 * @file audioTab.cpp
 * @brief Implementasi Audio Settings Tab
 * 
 * Handle rendering untuk pengaturan audio.
 */

#include "../include/audioTab.h"
#include "../lib/raylib/include/raylib.h"

void DrawAudioTab(
    Vector2 mousePosition,
    int startX,
    int startY)
{
    (void)mousePosition;
    int contentStartY = startY + 100;
    const int fontSize = 24;
    int labelX = startX + 40;
    int valueX = startX + 250;

    DrawText("Master Volume", labelX, contentStartY + 15, fontSize, WHITE);
    DrawText("100%", valueX, contentStartY + 15, fontSize, YELLOW);

    DrawText("Music Volume", labelX, contentStartY + 75, fontSize, WHITE);
    DrawText("80%", valueX, contentStartY + 75, fontSize, YELLOW);

    DrawText("SFX Volume", labelX, contentStartY + 135, fontSize, WHITE);
    DrawText("100%", valueX, contentStartY + 135, fontSize, YELLOW);

    DrawText("(Coming Soon)", labelX, contentStartY + 180, 18, GRAY);
}