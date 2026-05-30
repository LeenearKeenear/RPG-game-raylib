/**
 * @file keybindsTab.cpp
 * @brief Implementasi Keybinds Settings Tab
 * 
 * Handle rendering untuk daftar keybinds.
 */

#include "keybindsTab.h"
#include "../lib/raylib/include/raylib.h"
#include <array>

struct KeybindEntry {
    const char* key;
    const char* action;
};

static const std::array<KeybindEntry, 13> mainKeybinds = {{
    {"W / Arrow Up", "Move Up"},
    {"S / Arrow Down", "Move Down"},
    {"A / Arrow Left", "Move Left"},
    {"D / Arrow Right", "Move Right"},
    {"E", "Interact"},
    {"I", "Inventory"},
    {"M", "Map"},
    {"Q", "Drop Item"},
    {"Left Ctrl", "Drop All"},
    {"Mouse Left", "Attack"},
    {"Mouse Right", "Dash / Drink"},
    {"1 / 2 / 3 / 4", "Hotbar Slots"},
    {"Scroll", "Hotbar Slot"}
}};

static const std::array<KeybindEntry, 5> debugKeybinds = {{
    {"`", "Pause Menu"},
    {"Tab", "Debug Overlay"},
    {"R", "Revive"},
    {"K", "Damage (No Effect)"},
    {"B", "Previous Map"}
}};

/**
 * @brief Draw keybinds tab content
 * @param mousePosition Posisi mouse untuk efek hover
 * @param startX Posisi X awal area options
 * @param startY Posisi Y awal area options
 */
void DrawKeybindsTab(Vector2 mousePosition, int startX, int startY) {
    (void)mousePosition;
    int contentStartY = startY + 100;
    const int fontSize = 20;
    int col1X = startX + 40;
    int col2X = startX + 450;

    for (int i = 0; i < 9; i++) {
        int rowY = contentStartY + (i * 28);
        DrawText(mainKeybinds[i].key, col1X, rowY, fontSize, YELLOW);
        DrawText(" => ", col1X + 180, rowY, fontSize, GRAY);
        DrawText(mainKeybinds[i].action, col1X + 240, rowY, fontSize, WHITE);
    }

    for (int i = 0; i < 9; i++) {
        int srcIdx = 9 + i;
        int rowY = contentStartY + (i * 28);

        const KeybindEntry& entry = (srcIdx < 13)
            ? mainKeybinds[srcIdx]
            : debugKeybinds[srcIdx - 13];

        DrawText(entry.key, col2X, rowY, fontSize, YELLOW);
        DrawText(" => ", col2X + 80, rowY, fontSize, GRAY);
        DrawText(entry.action, col2X + 130, rowY, fontSize, WHITE);
    }
}
