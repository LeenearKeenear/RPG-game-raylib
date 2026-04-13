#include "../include/mainMenu.h"
#include "../include/popup.h"
#include "../lib/raylib/include/raymath.h"
#include <array>
#include <cstdint>

static std::array<buttonTxt, 4> buttons;
static Popup optionsPopup;

enum MenuButton : std::uint8_t {
    BTN_START = 0,
    BTN_LOAD,
    BTN_OPTIONS,
    BTN_QUIT
};

/**
 * Mengonversi koordinat mouse jendela ke koordinat layar virtual.
 */
static Vector2 GetVirtualMousePosition(GameState* state)
{
    Vector2 mouse = GetMousePosition();
    Vector2 virtualMouse = {0, 0};
    virtualMouse.x = (mouse.x - ((state->WindowScreenWidth - (GameScreenWidth * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;
    virtualMouse.y = (mouse.y - ((state->WindowScreenHeight - (GameScreenHeight * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;
    return Vector2Clamp(virtualMouse, (Vector2){0, 0}, (Vector2){(float)GameScreenWidth, (float)GameScreenHeight});
}

/**
 * @brief InitMainMenu()
 * Inisialisasi semua tombol menu utama dalam array.
 */
void InitMainMenu(GameState* state)
{
    std::array<const char*, 4> texts = {"Start Game", "Load Game", "Options", "Quit"};
    
    int centerX = (GameScreenWidth / 2) - 50;
    int startY = (GameScreenHeight / 2) - 100;
    int buttonSpacing = 70;
    int fontSize = 30;

    for (int i = 0; i < 4; i++) {
        buttons[i] = buttonTxt(texts[i], centerX, startY + (i * buttonSpacing), fontSize, WHITE, 0.6F);
    }

    optionsPopup = Popup("COMING SOON", "OK", 0.6F);
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

    if (optionsPopup.IsActive()) {
        optionsPopup.Update(mousePosition, mouseClicked);
        return;
    }

    for (int i = 0; i < 4; i++) {
        if (buttons[i].isClicked(mousePosition, mouseClicked)) {
            switch (i) {
                case BTN_START:
                    state->currentScreen = PLAY;
                    break;
                case BTN_OPTIONS:
                    optionsPopup.Show();
                    break;
                case BTN_QUIT:
                    CloseWindow();
                    break;
                case BTN_LOAD:
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

    if (optionsPopup.IsActive()) {
        optionsPopup.Draw(virtualMouse);
    }
    
    EndTextureMode();
}