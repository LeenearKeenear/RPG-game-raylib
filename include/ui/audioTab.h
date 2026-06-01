#pragma once

#include "../lib/raylib/include/raylib.h"
#include "buttonTxt.h"

/**
 * @file audioTab.h
 * @brief Audio Settings Tab for Options Screen
 * 
 * Contains volume controls and audio settings.
 */

static const char* AUDIO_SETTINGS_PATH = "saves/settings/audio.json";

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

/**
 * @brief Memuat pengaturan audio dari file JSON
 * @return true jika berhasil dimuat
 */
bool LoadAudioSettings();

/**
 * @brief Menyimpan pengaturan audio ke file JSON
 * @param masterVolume Volume master (0-100)
 * @param musicVolume Volume musik (0-100)
 * @param sfxVolume Volume SFX (0-100)
 * @return true jika berhasil disimpan
 */
bool SaveAudioSettings(int masterVolume, int musicVolume, int sfxVolume);