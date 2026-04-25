/**
 * @file main.cpp
 * @brief Entry Point Game Application
 *
 * Main game loop dan inisialisasi semua sistem.
 * Handle state management (MAIN_MENU, PLAY, OPTIONS) dan pause menu.
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

/**
 * @brief Global instances for menu systems
 */
PauseMenu pauseMenu;
OptionsScreen optionsScreen;

/**
 * @brief Main entry point game application
 * @return 0 saat game ditutup
 */
int main()
{
    // Inisialisasi
    // Urutan penting: InitScreen dulu → InitMap → InitAll → InitMainMenu
    // InitMap harus sebelum InitAll karena player butuh data map buat spawn

    // Step 1: buat window, audio, dan render texture virtual (1280x720)
    GameState state = InitScreen();
    state.previousScreen = MAIN_MENU; // Default return to main menu
    gState = &state;

    // Step 2: load map dari JSON Tiled
    InitMap();

    // Step 3: init player dan camera — spawn point dibaca dari map
    InitAll();

    // Step 4: init elemen UI main menu
    InitMainMenu(&state);

    // Step 5: init options screen (hidden initially)
    optionsScreen.Show();
    optionsScreen.Hide();

    // Main Game Loop
    while (!WindowShouldClose())
    {
        // State: MAIN_MENU
        if (state.currentScreen == MAIN_MENU)
        {
            UpdateGame(&state);
            UpdateMainMenu(&state);
            RenderMainMenuToVirtualScreen(&state);
            DrawRenderWindows(&state);
        }
        // State: OPTIONS
        else if (state.currentScreen == OPTIONS)
        {
            // Show options screen and set return screen on first entry
            if (!optionsScreen.IsActive()) {
                optionsScreen.SetReturnScreen(state.previousScreen);
                optionsScreen.Show();
            }

            UpdateGame(&state);
            bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
            optionsScreen.Update(&state, GetVirtualMousePosition(&state), mouseClicked);

            BeginTextureMode(state.Dungeon);
            ClearBackground(DARKGRAY);
            optionsScreen.Draw(GetVirtualMousePosition(&state));
            EndTextureMode();

            DrawRenderWindows(&state);
        }
        // State: PLAY
        else if (state.currentScreen == PLAY)
        {
            // If returning from OPTIONS, ensure pause menu is shown
            if (state.previousScreen == OPTIONS && !pauseMenu.IsActive()) {
                pauseMenu.Show();
            }

            // toggle pause menu dengan tombol P
            if (IsKeyPressed(KEY_P))
            {
                if (pauseMenu.IsActive())
                {
                    pauseMenu.Hide();
                }
                else
                {
                    pauseMenu.Show();
                }
            }

            // update scale sebelum rendering
            UpdateGame(&state);

            // capture mouse click before rendering
            bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

            // update pause menu if active (HARUS sebelum rendering)
            if (pauseMenu.IsActive())
            {
                pauseMenu.Update(&state, GetVirtualMousePosition(&state), mouseClicked);
            }

            // update semua logic game - skip when paused
            if (!pauseMenu.IsActive()) {
                UpdateLogicAll();
            }

            // render semua ke layar virtual
            DrawRenderTexture(&state);

            // scale layar virtual ke window asli
            DrawRenderWindows(&state);
        }
    }

    // Shutdown
    GameShutDown(&state);
    return 0;
}