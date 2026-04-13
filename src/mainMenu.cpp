#include "../include/mainMenu.h"
#include "../lib/raylib/include/raymath.h"

static buttonTxt startButton;
static buttonTxt loadgameButton;
static buttonTxt optionsButton;
static buttonTxt quitButton;

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
 * Menginisialisasi tombol menu utama.
 * Membuat tombol "Start Game" dan "Quit" yang centered di layar virtual.
 */
void InitMainMenu(GameState* state)
{
    int centerX = (GameScreenWidth / 2) - 50;
    int buttonSpacing = 100;
    int fontSize = 30;

    // Game State
    startButton = buttonTxt("Start Game", centerX, 300, fontSize, WHITE, 0.6F);
    quitButton = buttonTxt("Quit", centerX, 300 + buttonSpacing, fontSize, WHITE, 0.6F);

    // Load Game
    loadgameButton = buttonTxt("Load Game", centerX, 300, fontSize, WHITE, 0.6F);

    // Options menu
    optionsButton = buttonTxt("Options", centerX, 300 + buttonSpacing, fontSize, WHITE, 0.6F);

}

/**
 * Mengupdate state menu utama - menangani input mouse dan pendeteksian klik tombol.
 * Menggunakan koordinat layar virtual untuk mendeteksi klik tombol dengan benar
 * terlepas dari ukuran/skala jendela.
 */
void UpdateMainMenu(GameState* state)
{
    Vector2 mousePosition = GetVirtualMousePosition(state);
    bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    // Start Game
    if (startButton.isClicked(mousePosition, mouseClicked))
    {
        state->currentScreen = PLAY;
    }

    // Close game
    if (quitButton.isClicked(mousePosition, mouseClicked))
    {
        CloseWindow();
    }

    // TODO Load Game

    // Options
    if (optionsButton.isClicked(mousePosition, mouseClicked))
    {
        state->currentScreen = OPTIONS;
    }
}

/**
 * Merender menu utama ke layar virtual (RenderTexture2D).
 * Ini menggambar ke buffer off-screen 1280x720 yang kemudian diskalakan
 * untuk menyesuaikan jendela oleh DrawRenderWindows().
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