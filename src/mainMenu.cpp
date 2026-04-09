#include "../include/mainMenu.h"
#include "../lib/raylib/include/raymath.h"

static buttonTxt startButton;
static buttonTxt quitButton;

/**
 * Converts window mouse coordinates to virtual screen coordinates.
 * The virtual screen is 1280x720 (GameScreenWidth x GameScreenHeight) and
 * gets scaled to fit the window while maintaining aspect ratio.
 *
 * @param state Pointer to the current game state containing window/scale info
 * @return Vector2 position in virtual screen coordinates (0-1280, 0-720)
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
 * Initializes the main menu buttons.
 * Creates "Start Game" and "Quit" buttons centered on the virtual screen.
 *
 * @param state Pointer to the game state (unused, reserved for future use)
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
 * Updates main menu state - handles mouse input and button click detection.
 * Uses virtual screen coordinates to correctly detect button clicks regardless
 * of window size/scaling.
 *
 * @param state Pointer to the current game state
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
 * Renders the main menu to the virtual screen (RenderTexture2D).
 * This draws to an off-screen 1280x720 buffer which is then scaled
 * to fit the window by DrawRenderWindows().
 *
 * @param state Pointer to the current game state
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