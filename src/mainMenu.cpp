#include "../include/mainMenu.h"
#include "../lib/raylib/include/raymath.h"

static buttonTxt startButton;
static buttonTxt quitButton;

/**
 * Mengonversi koordinat mouse jendela ke koordinat layar virtual.
 * Layar virtual adalah 1280x720 (GameScreenWidth x GameScreenHeight) dan
 * akan diskalakan untuk menyesuaikan jendela sambil mempertahankan rasio aspek.
 *
 * @param state Pointer ke state game saat ini yang berisi info jendela/skala
 * @return Vector2 posisi dalam koordinat layar virtual (0-1280, 0-720)
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
 * Menginisialisasi tombol menu utama.
 * Membuat tombol "Start Game" dan "Quit" yang centered di layar virtual.
 *
 * @param state Pointer ke state game (tidak digunakan, reserved untuk penggunaan masa depan)
 */
void InitMainMenu(GameState* state)
{
    int centerX = (GameScreenWidth / 2) - 50;
    int buttonSpacing = 50;
    int fontSize = 30;

    startButton = buttonTxt("Start Game", centerX, 300, fontSize, WHITE, 0.6F);
    quitButton = buttonTxt("Quit", centerX, 300 + buttonSpacing, fontSize, WHITE, 0.6F);
}

/**
 * Mengupdate state menu utama - menangani input mouse dan pendeteksian klik tombol.
 * Menggunakan koordinat layar virtual untuk mendeteksi klik tombol dengan benar
 * terlepas dari ukuran/skala jendela.
 *
 * @param state Pointer ke state game saat ini
 */
void UpdateMainMenu(GameState* state)
{
    Vector2 mousePosition = GetVirtualMousePosition(state);
    bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    if (startButton.isClicked(mousePosition, mouseClicked))
    {
        state->currentScreen = PLAY;
    }

    if (quitButton.isClicked(mousePosition, mouseClicked))
    {
        CloseWindow();
    }
}

/**
 * Merender menu utama ke layar virtual (RenderTexture2D).
 * Ini menggambar ke buffer off-screen 1280x720 yang kemudian diskalakan
 * untuk menyesuaikan jendela oleh DrawRenderWindows().
 *
 * @param state Pointer ke state game saat ini
 */
void RenderMainMenuToVirtualScreen(GameState* state)
{
    Vector2 virtualMouse = GetVirtualMousePosition(state);
    BeginTextureMode(state->Dungeon);
    ClearBackground(DARKGRAY);
    startButton.Draw(virtualMouse);
    quitButton.Draw(virtualMouse);
    EndTextureMode();
}