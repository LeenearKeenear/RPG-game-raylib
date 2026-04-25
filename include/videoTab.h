#pragma once

#include "../lib/raylib/include/raylib.h"
#include "buttonTxt.h"

/**
 * @file videoTab.h
 * @brief Video Settings Tab for Options Screen
 * 
 * Contains fullscreen toggle and FPS display toggle.
 */

/**
 * @brief Me-render tab Video
 * @param fullscreenButton Tombol toggle fullscreen
 * @param fpsButton Tombol toggle FPS display
 * @param mousePosition Posisi mouse untuk efek hover
 * @param startX Posisi X awal area options
 * @param startY Posisi Y awal area options
 */
void DrawVideoTab(
    buttonTxt& fullscreenButton,
    buttonTxt& fpsButton,
    Vector2 mousePosition,
    int startX,
    int startY
);

/**
 * @brief Memperbarui handling input untuk tab Video
 * @param fullscreenButton Tombol toggle fullscreen
 * @param fpsButton Tombol toggle FPS display
 * @param state Pointer ke GameState
 * @param mousePosition Posisi mouse saat ini
 * @param mouseClicked Status klik mouse
 * @return true jika ada perubahan yang memerlukan redraw
 */
bool UpdateVideoTab(
    buttonTxt& fullscreenButton,
    buttonTxt& fpsButton,
    void* state,
    Vector2 mousePosition,
    bool mouseClicked
);
