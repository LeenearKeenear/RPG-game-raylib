#include "../include/screen.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../include/mainMenu.h"
<<<<<<< HEAD
#include "../include/enemy.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"

=======
#include "../include/pauseMenu.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"

PauseMenu pauseMenu;
>>>>>>> ed566672fef6c48fd5aac57fac0126a3648037c9

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

    //Init Enemy buat test, nanti dibenerin
    EnemyInitTest();

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
            // toggle pause menu dengan P key
            if (IsKeyPressed(KEY_P)) {
                if (pauseMenu.IsActive()) {
                    pauseMenu.Hide();
                } else {
                    pauseMenu.Show();
                }
            }

            // update scale sebelum rendering (needed for mouse position calculation)
            UpdateGame(&state);

            // capture mouse click before rendering
            bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

            // update pause menu if active (MUST be before rendering)
            if (pauseMenu.IsActive()) {
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