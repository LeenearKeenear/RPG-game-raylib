#include "../include/screen.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../include/mainMenu.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"

int main()
{
    // Inisialisasi jendela game dan sistem rendering
    // Membuat layar virtual (1280x720) yang diskalakan untuk menyesuaikan jendela sebenarnya
    GameState state = InitScreen();

    // Inisialisasi posisi player dan entity game lainnya
    TilesonInit(&state);
    InitAll();

    // Inisialisasi elemen UI menu utama (tombol, dll)
    InitMainMenu(&state);

    // Main game loop - berjalan setiap frame sampai jendela ditutup
    while (!WindowShouldClose())
    {
        // State MAIN_MENU - menampilkan layar awal dengan tombol
        if (state.currentScreen == MAIN_MENU)
        {
            // UpdateGame menangani event resize jendela dan mengupdate ScaleMultiplier
            // Ini memastikan menu diskalakan dengan benar saat ukuran jendela berubah
            UpdateGame(&state);
            // Menangani klik tombol dan transisi
            UpdateMainMenu(&state);
            // Menggambar tombol ke layar virtual (1280x720 RenderTexture)
            RenderMainMenuToVirtualScreen(&state);
            // Menggambar layar virtual ke jendela, diskalakan untuk menyesuaikan sambil mempertahankan rasio aspek
            DrawRenderWindows(&state);
        }
        // State PLAY - gameplay sebenarnya
        else if (state.currentScreen == PLAY)
        {
            // Update info jendela/skala dan logika game
            UpdateGame(&state);
            // update logic buat semuanya
            UpdateLogicAll(&state);
            // Merender game world ke layar virtual
            DrawRenderTexture(&state);
            // Menggambar layar virtual ke jendela dengan scaling yang tepat
            DrawRenderWindows(&state);
        }
    }

    // Clean up resource (textures, maps, window, dll)
    GameShutDown(&state);
    return 0;
}