#pragma once

/**
 * @file screen.h
 * @brief Screen & GameState Management Module
 *
 * Handle virtual screen (1280x720), scaling, dan state management.
 * Semua rendering pake virtual screen biar konsisten di berbagai resolusi.
 */

#include "../lib/raylib/include/raylib.h"

// ================================================================
// Screen & GameState
// Handle virtual screen (1280x720), scaling, dan state management.
//
// Cara kerja virtual screen:
// - Semua rendering ke RenderTexture2D (layar virtual 1280x720)
// - Layar virtual di-scale ke window asli sambil jaga aspect ratio
// - ScaleMultiplier dihitung tiap frame buat handle window resize
// ================================================================

/*==============================================================================
 * Virtual Screen Constants
 *==============================================================================*/

/** Ukuran layar virtual — semua rendering mengacu ke ukuran ini */
extern const int GameScreenWidth;
extern const int GameScreenHeight;

/*==============================================================================
 * ScreenState Enum
 *==============================================================================*/

/**
 * @brief State game yang aktif
 * @note Tambah state baru di sini kalo perlu (misal SAVE_MENU, LOAD_MENU)
 */
typedef enum
{
    // General States
    MAIN_MENU, /**< State main menu — nampilin menu utama */
    PLAY,      /**< State play — lagi main game */

    // TODO: Save states (nanti tambahin kalo udah implementasi save/load)

    // Extras
    OPTIONS /**< State options — menu pengaturan game */
} ScreenState;

/*==============================================================================
 * GameState Struct
 *==============================================================================*/

/**
 * @brief Struct utama yang nyimpen semua info rendering dan state game
 */
typedef struct
{
    RenderTexture2D Dungeon;   /**< Render texture virtual (1280x720) - target rendering semua game */
    float ScaleMultiplier;     /**< Rasio scale layar virtual ke window asli (dihitung tiap frame) */
    int WindowScreenWidth;     /**< Ukuran window asli saat ini (bisa berubah kalo resize) */
    int WindowScreenHeight;    /**< Ukuran window asli saat ini (bisa berubah kalo resize) */
    ScreenState currentScreen; /**< State game yang aktif (MAIN_MENU / PLAY / OPTIONS) */
} GameState;

extern GameState *gState;

/*==============================================================================
 * Screen Functions
 *==============================================================================*/

/**
 * @brief Inisialisasi window, audio, dan render texture virtual
 * @return GameState yang udah diinisialisasi
 * @note Panggil sekali pas awal game start
 */
GameState InitScreen(void);

/**
 * @brief Update ukuran window dan scale multiplier
 * @param state Pointer ke GameState
 * @note Dipanggil tiap frame buat handle window resize
 */
void UpdateGame(GameState *state);

/**
 * @brief Inisialisasi semua entity dan camera
 * @note Panggil setelah InitScreen() buat setup player, map, dll
 */
void InitAll(void);

/**
 * @brief Entry point render — semua rendering ke layar virtual lewat sini
 * @param state Pointer ke GameState
 * @note Manggil fungsi-fungsi render berdasarkan currentScreen
 */
void DrawRenderTexture(GameState *state);

/**
 * @brief UI overlay rendering (pause menu, dll) ke virtual screen
 * @param state Pointer ke GameState
 * @note Render di atas game screen
 */
void DrawUIOverlay(GameState *state);

/**
 * @brief Konversi koordinat mouse dari window ke virtual screen
 * @param state Pointer ke GameState (butuh ScaleMultiplier)
 * @return Vector2 posisi mouse dalam koordinat virtual screen
 * @note Dipake buat deteksi klik UI yang dirender di virtual screen
 */
Vector2 GetVirtualMousePosition(GameState *state);

/**
 * @brief Entry point logic — semua logic game per frame lewat sini
 * @note Manggil fungsi-fungsi update berdasarkan currentScreen
 */
void UpdateLogicAll(void);

/**
 * @brief Scale layar virtual ke window asli
 * @param state Pointer ke GameState
 * @note Render texture virtual di-draw ke window asli dengan scale dan letterbox
 */
void DrawRenderWindows(GameState *state);

/**
 * @brief Bersihin semua resource sebelum game ditutup
 * @param state Pointer ke GameState
 * @note Unload texture, render texture, dan close window
 */
void GameShutDown(GameState *state);