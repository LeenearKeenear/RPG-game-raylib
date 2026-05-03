#pragma once

/**
 * @file screen.h
 * @brief Screen & GameState Management Module
 *
 * Header ini mendeklarasikan sistem virtual screen, scaling window,
 * dan state utama game untuk proses update serta rendering.
 */

#include "../lib/raylib/include/raylib.h"
#include <atomic>
#include <cstdint>
#include <string>

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
using ScreenState = enum : std::uint8_t
{
    MAIN_MENU, // State menu utama
    LOADING,   // State loading aset
    PLAY,      // State gameplay aktif
    OPTIONS    // State menu pengaturan
};

/*==============================================================================
 * GameState Struct
 *==============================================================================*/

/**
 * @brief Menyimpan data utama rendering dan state game
 */
using GameState = struct
{
    RenderTexture2D Dungeon;   /**< Render texture virtual (1280x720) - target rendering semua game */
    float ScaleMultiplier;     /**< Rasio scale layar virtual ke window asli (dihitung tiap frame) */
    int WindowScreenWidth;     /**< Ukuran window asli saat ini (bisa berubah kalo resize) */
    int WindowScreenHeight;    /**< Ukuran window asli saat ini (bisa berubah kalo resize) */
    ScreenState currentScreen; /**< State game yang aktif (MAIN_MENU / PLAY / OPTIONS) */
    ScreenState previousScreen;/**< Screen sebelum OPTIONS - buat return button */
    bool showFPS;              /**< Tampilkan FPS counter di HUD */
    
/*==============================================================================
     * Loading State Variables
     *==============================================================================*/
    /**
     * @brief Progress loading saat ini (0-100)
     */
    float loadingProgress;
    /**
     * @brief Teks status loading yang ditampilkan
     */
    const char* loadingText;
    /**
     * @brief Flag menandakan loading sudah selesai
     */
    bool loadingComplete;
    /**
     * @brief Stage loading saat ini (reset setiap masuk LOADING state)
     * @details Dipakai untuk tracking switch statement di UpdateLoadingScreen()
     */
    int loadingStage;
    /**
     * @brief Flag menandakan asset sudah dimuat ke memori
     * @details Jika true, Start Game berikutnya akan skip loading texture
     */
    bool assetsLoaded;
    /**
     * @brief Flag menandakan sudah pernah masuk LOADING state
     * @details Dipakai agar InitLoadingScreen tidak dipanggil setiap frame
     */
    bool enteredLoading;

    /*==============================================================================
     * Map Switch State Variables
     *==============================================================================*/
    /**
     * @brief Flag menandakan sedang dalam proses switch map
     * @details Dipakai oleh loading screen untuk tahu harus load map baru
     */
    bool isSwitchingMap;
    /**
     * @brief Path map tujuan yang akan dimuat
     */
    std::string pendingMapPath;
    /**
     * @brief Nama door atau spawn point tujuan
     */
    std::string pendingDoorName;
};

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
