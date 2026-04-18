#pragma once
#include "raylib.h"
#include "screen.h"

/**
 * @brief Render HUD player (Nama, Health, Mana) di pojok kiri bawah.
 * Fungsi ini dipanggil dari DrawUIOverlay di screen_handler.cpp
 */
void DrawPlayerHUD();
