#include "../include/screen.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../include/mainMenu.h"
#include "../include/pauseMenu.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"

PauseMenu pauseMenu;

int main()
{
    // ================================================================
    // Inisialisasi
    // Urutan penting: InitScreen dulu → InitMap → InitAll → InitMainMenu
    // InitMap harus sebelum InitAll karena player butuh data map buat spawn
    // ================================================================

    // buat window, audio, dan render texture virtual (1280x720)
    GameState state = InitScreen();

    // load map dari JSON Tiled
    InitMap();

    // init player dan camera — spawn point dibaca dari map
    InitAll();

    // init elemen UI main menu
    InitMainMenu(&state);

    // ================================================================
    // Main Game Loop
    // ================================================================
    while (!WindowShouldClose())
    {
        // state MAIN_MENU — tampilan awal dengan tombol start & quit
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
        // state PLAY — gameplay
        else if (state.currentScreen == PLAY)
        {
            // toggle pause menu dengan P key (not ESC)
            if (IsKeyPressed(KEY_P)) {
                TraceLog(LOG_INFO, "P key pressed!");
                if (pauseMenu.IsActive()) {
                    TraceLog(LOG_INFO, "Hiding pause menu...");
                    pauseMenu.Hide();
                } else {
                    TraceLog(LOG_INFO, "Showing pause menu...");
                    pauseMenu.Show();
                }
            }

            // update scale kalau window di-resize
            UpdateGame(&state);
            // update semua logic game (player, enemy, dll)
            UpdateLogicAll();
            // render semua ke layar virtual
            DrawRenderTexture(&state);
            // scale layar virtual ke window asli
            DrawRenderWindows(&state);

            // update pause menu jika aktif
            if (pauseMenu.IsActive()) {
                pauseMenu.Update(&state, GetVirtualMousePosition(&state), IsMouseButtonPressed(MOUSE_LEFT_BUTTON));
            }
        }
    }

    // ================================================================
    // Shutdown — bersihin semua resource sebelum tutup
    // ================================================================
    GameShutDown(&state);
    return 0;
}