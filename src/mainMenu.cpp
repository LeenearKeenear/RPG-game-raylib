#include "../include/mainMenu.h"
#include "../include/popup.h"
#include "../include/screen.h"
#include "../lib/raylib/include/raymath.h"
#include <array>
#include <cstdint>

static std::array<buttonTxt, 4> buttons;
static Popup menuOptionsPopup;

/**
 * @brief InitMainMenu()
 * Inisialisasi semua tombol menu utama dalam array.
 */
void InitMainMenu(GameState* state)
{
    (void)state;

    std::array<const char*, 4> texts = {"Start Game", "Load Game", "Options", "Quit"};
    
    int centerX = (GameScreenWidth / 2) - 50;
    int startY = (GameScreenHeight / 2) - 100;
    int buttonSpacing = 70;
    int fontSize = 30;

    for (int i = 0; i < 4; i++) {
        buttons[i] = buttonTxt(texts[i], centerX, startY + (i * buttonSpacing), fontSize, WHITE, 0.6F);
    }

    menuOptionsPopup = Popup("COMING SOON", "OK", 0.6F);
}

/**
 * @brief UpdateMainMenu()
 * Tangani input mouse dan klik tombol untuk navigasi menu.
 * Popup bersifat modal - blokir interaksi menu saat popup aktif.
 */
void UpdateMainMenu(GameState* state)
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
 */
void RenderMainMenuToVirtualScreen(GameState* state)
{
    Vector2 virtualMouse = GetVirtualMousePosition(state);
    BeginTextureMode(state->Dungeon);
    ClearBackground(DARKGRAY);
    
    for (int i = 0; i < 4; i++) {
        buttons[i].Draw(virtualMouse);
    }

    if (menuOptionsPopup.IsActive()) {
        menuOptionsPopup.Draw(virtualMouse);
    }
    
    EndTextureMode();
}