#pragma once
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

// ukuran layar virtual — semua rendering mengacu ke ukuran ini
extern const int GameScreenWidth;
extern const int GameScreenHeight;

// enum state game — tambah state baru di sini kalau perlu
typedef enum
{
    // General States
    MAIN_MENU,
    PLAY,

    // TODO Save staes

    // Extras
    OPTIONS
} ScreenState;

// struct utama yang nyimpen semua info rendering dan state game
typedef struct
{
    RenderTexture2D Dungeon; // render texture virtual (1280x720)
    float ScaleMultiplier;   // rasio scale layar virtual ke window asli
    int WindowScreenWidth;   // ukuran window asli saat ini
    int WindowScreenHeight;
    ScreenState currentScreen; // state game yang aktif
} GameState;

// inisialisasi window, audio, dan render texture virtual
GameState InitScreen(void);

// update ukuran window dan scale multiplier — dipanggil tiap frame
void UpdateGame(GameState *state);

// inisialisasi semua entity dan camera
void InitAll(void);

// entry point render — semua rendering ke layar virtual lewat sini
void DrawRenderTexture(GameState *state);

// UI overlay rendering (pause menu, etc) ke virtual screen
void DrawUIOverlay(GameState *state);

// entry point logic — semua logic game per frame lewat sini
void UpdateLogicAll(void);

// scale layar virtual ke window asli
void DrawRenderWindows(GameState *state);

// bersihin semua resource sebelum game ditutup
void GameShutDown(GameState *state);