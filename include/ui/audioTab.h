#pragma once

#include "../lib/raylib/include/raylib.h"
#include "buttonTxt.h"

/**
 * @file audioTab.h
 * @brief Audio Settings Tab for Options Screen
 * 
 * Contains volume controls and audio settings.
 */

/**
 * @brief Me-render tab Audio
 * @param mousePosition Posisi mouse untuk efek hover
 * @param startX Posisi X awal area options
 * @param startY Posisi Y awal area options
 */
void DrawAudioTab(
    Vector2 mousePosition,
    int startX,
    int startY
);