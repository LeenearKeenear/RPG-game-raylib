/**
 * @file mainMenu.cpp
 * @brief Implementasi dari Main Menu System Module
 *
 * Implementasi dari fungsi-fungsi main menu yang dideklarasikan di mainMenu.h
 * Handle inisialisasi tombol, update input, dan rendering menu utama.
 */

#include "../include/mainMenu.h"
#include "../include/popup.h"
#include "../include/screen.h"
#include "../lib/raylib/include/raymath.h"
#include <array>
#include <cstdint>

/*==============================================================================
 * Static Variables
 *==============================================================================*/

/** Array tombol menu utama (Start, Load, Options, Quit) */
static std::array<buttonTxt, 4> buttons;

/** Popup buat fitur "Coming Soon" (Options & Load sementara) */
static Popup menuOptionsPopup;

/*==============================================================================
 * Public Functions
 *==============================================================================*/

/**
 * @brief InitMainMenu()
 * Inisialisasi semua tombol menu utama dalam array.
 * @param state GameState pointer (dipake buat akses screen size, tapi sementara di-ignore)
 */
void InitMainMenu(GameState *state)
{
    (void)state; // unused parameter, buat future use

    // Daftar teks tombol sesuai urutan enum MenuButton
    std::array<const char *, 4> texts = {"Start Game", "Load Game", "Options", "Quit"};

    // Hitung posisi tengah layar virtual
    int centerX = (GameScreenWidth / 2) - 50;
    int startY = (GameScreenHeight / 2) - 100;
    int buttonSpacing = 70;
    int fontSize = 30;

    // Looping bikin 4 tombol dengan posisi vertikal berurutan
    for (int i = 0; i < 4; i++)
    {
        buttons[i] = buttonTxt(texts[i], centerX, startY + (i * buttonSpacing), fontSize, WHITE, 0.6F);
    }

    menuOptionsPopup = Popup("COMING SOON", "OK", 0.6F);
}

/**
 * @brief UpdateMainMenu()
 * Tangani input mouse dan klik tombol untuk navigasi menu.
 * @param state GameState pointer - buat ngubah currentScreen
 * @note Popup bersifat modal - blokir interaksi menu saat popup aktif
 */
void UpdateMainMenu(GameState *state)
{
    Vector2 mousePosition = GetVirtualMousePosition(state);
    bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    if (menuOptionsPopup.IsActive()) {
        menuOptionsPopup.Update(mousePosition, mouseClicked);
        return;
    }

    for (int i = 0; i < 4; i++) {
        if (buttons[i].isClicked(mousePosition, mouseClicked)) {
            switch (i) {
                case 0:  // Start Game
                    state->currentScreen = PLAY;
                    break;
                case 2:  // Options
                    menuOptionsPopup.Show();
                    break;
                case 3:  // Quit
                    CloseWindow();
                    break;
                case 1:  // Load Game
                default:
                    break;
            }
        }
    }
}

/**
 * @brief RenderMainMenuToVirtualScreen()
 * Render semua tombol menu ke layar virtual.
 * @param state GameState pointer - buat akses ke render texture
 */
void RenderMainMenuToVirtualScreen(GameState *state)
{
    Vector2 virtualMouse = GetVirtualMousePosition(state);

    // Mulai render ke texture virtual
    BeginTextureMode(state->Dungeon);
    ClearBackground(DARKGRAY);

    // Render semua tombol menu
    for (int i = 0; i < 4; i++)
    {
        buttons[i].Draw(virtualMouse);
    }

    if (menuOptionsPopup.IsActive()) {
        menuOptionsPopup.Draw(virtualMouse);
    }

    EndTextureMode();
}