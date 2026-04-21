/**
 * @file main.cpp
 * @brief Entry Point Game Application
 *
 * Main game loop dan inisialisasi semua sistem.
 * Handle state management (MAIN_MENU, PLAY) dan pause menu.
 */

#include "../include/screen.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../include/enemy.h"
#include "../include/item.h"
#include "../include/mainMenu.h"
#include "../include/pauseMenu.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"

/*==============================================================================
 * Global Variables
 *==============================================================================*/

/** Global instance pause menu — bisa diakses dari mana aja */
PauseMenu pauseMenu;

/*==============================================================================
 * Main Function
 *==============================================================================*/

int main()
{
    // ================================================================
    // Inisialisasi
    // Urutan penting: InitScreen dulu → InitMap → InitAll → InitMainMenu
    // InitMap harus sebelum InitAll karena player butuh data map buat spawn
    // ================================================================

    // Step 1: buat window, audio, dan render texture virtual (1280x720)
    GameState state = InitScreen();

    // Step 2: load map dari JSON Tiled
    InitMap();

    // Step 3: init player dan camera — spawn point dibaca dari map
    InitAll();

    // Step 4: init elemen UI main menu
    InitMainMenu(&state);

    //Step 5: init enemy
    InitEnemy();

    //Testing spawn item
    SpawnPotion({750, 169}, 3, RARITY_COMMON);

    // ================================================================
    // Main Game Loop
    // ================================================================
    while (!WindowShouldClose())
    {
        // ===== State: MAIN_MENU =====
        // Tampilan awal dengan tombol start & quit
        if (state.currentScreen == MAIN_MENU)
        {
            // update scale kalau window di-resize
            UpdateGame(&state);

            // handle input tombol dan transisi ke state lain
            UpdateMainMenu(&state);

            // render menu ke layar virtual
            RenderMainMenuToVirtualScreen(&state);

            // scale layar virtual ke window asli
            DrawRenderWindows(&state);
        }
        // ===== State: PLAY =====
        // Gameplay aktif
        else if (state.currentScreen == PLAY)
        {
            // toggle pause menu dengan tombol P
            if (IsKeyPressed(KEY_P))
            {
                if (pauseMenu.IsActive())
                {
                    pauseMenu.Hide(); // kalo lagi aktif, sembunyiin
                }
                else
                {
                    pauseMenu.Show(); // kalo gak aktif, tampilin
                }
            }

            // update scale sebelum rendering (dibutuhkan buat kalkulasi posisi mouse)
            UpdateGame(&state);

            // capture mouse click before rendering
            bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

            // update pause menu if active (HARUS sebelum rendering)
            if (pauseMenu.IsActive())
            {
                pauseMenu.Update(&state, GetVirtualMousePosition(&state), mouseClicked);
            }

            // update semua logic game (player, enemy, dll)
            UpdateLogicAll();

            // render semua ke layar virtual
            DrawRenderTexture(&state);

            // scale layar virtual ke window asli
            DrawRenderWindows(&state);
        }
    }

    // ================================================================
    // Shutdown — bersihin semua resource sebelum tutup
    // ================================================================
    GameShutDown(&state);
    return 0;
}