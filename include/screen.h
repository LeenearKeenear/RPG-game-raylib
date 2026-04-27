#pragma once

/**
 * @file screen.h
 * @brief Screen & GameState Management Module
 *
 * Header ini mendeklarasikan sistem virtual screen, scaling window,
 * dan state utama game untuk proses update serta rendering.
 */

#include "../lib/raylib/include/raylib.h"

/*==============================================================================
 * Virtual Screen Constants
 *==============================================================================*/

// Ukuran layar virtual yang dipakai seluruh proses rendering
extern const int GameScreenWidth;
extern const int GameScreenHeight;

/*==============================================================================
 * ScreenState Enum
 *==============================================================================*/

/**
 * @brief Daftar state utama game
 */
typedef enum
{
    MAIN_MENU, // State menu utama
    PLAY,      // State gameplay aktif
    OPTIONS    // State menu pengaturan
} ScreenState;

/*==============================================================================
 * GameState Struct
 *==============================================================================*/

/**
 * @brief Menyimpan data utama rendering dan state game
 */
typedef struct
{
    RenderTexture2D Dungeon;   /**< Render texture virtual (1280x720) - target rendering semua game */
    float ScaleMultiplier;     /**< Rasio scale layar virtual ke window asli (dihitung tiap frame) */
    int WindowScreenWidth;     /**< Ukuran window asli saat ini (bisa berubah kalo resize) */
    int WindowScreenHeight;    /**< Ukuran window asli saat ini (bisa berubah kalo resize) */
    ScreenState currentScreen; /**< State game yang aktif (MAIN_MENU / PLAY / OPTIONS) */
    ScreenState previousScreen;/**< Screen sebelum OPTIONS - buat return button */
    bool showFPS;              /**< Tampilkan FPS counter di HUD */
} GameState;

// Pointer global ke GameState aktif
extern GameState *gState;

/*==============================================================================
 * Screen Functions
 *==============================================================================*/

/**
 * @brief Inisialisasi window, audio, dan virtual screen
 * @return GameState yang sudah siap dipakai
 */
GameState InitScreen(void);

/**
 * @brief Update ukuran window dan nilai scale multiplier
 * @param state Pointer ke GameState aktif
 */
void UpdateGame(GameState *state);

/**
 * @brief Inisialisasi entity dan camera di awal game
 */
void InitAll(void);

/**
 * @brief Render seluruh isi game ke virtual screen
 * @param state Pointer ke GameState aktif
 */
void DrawRenderTexture(GameState *state);

/**
 * @brief Render elemen UI di atas virtual screen
 * @param state Pointer ke GameState aktif
 */
void DrawUIOverlay(GameState *state);
void DrawPlayerHUD();

/**
 * @brief Konversi posisi mouse dari window space ke virtual screen space
 * @param state Pointer ke GameState aktif
 * @return Posisi mouse dalam koordinat virtual screen
 */
Vector2 GetVirtualMousePosition(GameState *state);

/**
 * @brief Jalankan seluruh update logic game per frame
 */
void UpdateLogicAll(void);
void SpawnEnemiesFromMap(void);

/**
 * @brief Gambar virtual screen ke window asli dengan scaling
 * @param state Pointer ke GameState aktif
 */
void DrawRenderWindows(GameState *state);

/**
 * @brief Bersihkan resource sebelum game ditutup
 * @param state Pointer ke GameState aktif
 */
void GameShutDown(GameState *state);

/*==============================================================================
 * Window & Video Settings Functions
 *==============================================================================*/

/**
 * @brief ToggleFullscreenMode()
 * Toggle antara fullscreen dan windowed mode.
 */
void ToggleFullscreenMode(void);

/**
 * @brief SetResolution()
 * Set ukuran window ke resolusi tertentu.
 * @param width Lebar window baru
 * @param height Tinggi window baru
 */
void SetResolution(int width, int height);

/**
 * @brief GetCurrentResolution()
 * Ambil resolusi saat ini.
 * @return Rectangle berisi width dan height
 */
Rectangle GetCurrentResolution(void);

/**
 * @brief GetMonitorResolution()
 * Ambil resolusi monitor utama.
 * @return Rectangle berisi width dan height monitor
 */
Rectangle GetMonitorResolution(void);

/**
 * @brief IsFullscreen()
 * Cek apakah sedang dalam mode fullscreen.
 * @return true kalo fullscreen
 */
bool IsFullscreen(void);
