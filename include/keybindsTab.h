#pragma once

#include "../lib/raylib/include/raylib.h"

/**
 * @file keybindsTab.h
 * @brief Keybinds Settings Tab for Options Screen
 * 
 * Contains keybind display and controls.
 */

/**
 * @brief Me-render tab Keybinds
 * @param mousePosition Posisi mouse untuk efek hover
 * @param startX Posisi X awal area options
 * @param startY Posisi Y awal area options
 */
void DrawKeybindsTab(
    Vector2 mousePosition,
    int startX,
    int startY
);