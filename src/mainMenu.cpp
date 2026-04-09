#include "../include/mainMenu.h"
#include "../lib/raylib/include/raymath.h"

static buttonTxt startButton;
static buttonTxt quitButton;

static Vector2 GetVirtualMousePosition(GameState* state)
{
    Vector2 mouse = GetMousePosition();
    Vector2 virtualMouse = {0, 0};
    virtualMouse.x = (mouse.x - ((state->WindowScreenWidth - (GameScreenWidth * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;
    virtualMouse.y = (mouse.y - ((state->WindowScreenHeight - (GameScreenHeight * state->ScaleMultiplier)) * 0.5F)) / state->ScaleMultiplier;
    return Vector2Clamp(virtualMouse, (Vector2){0, 0}, (Vector2){(float)GameScreenWidth, (float)GameScreenHeight});
}

void InitMainMenu(GameState* state)
{
    int centerX = (GameScreenWidth / 2) - 50;
    int buttonSpacing = 50;
    int fontSize = 30;

    startButton = buttonTxt("Start Game", centerX, 300, fontSize, WHITE, 0.6F);
    quitButton = buttonTxt("Quit", centerX, 300 + buttonSpacing, fontSize, WHITE, 0.6F);
}

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

void RenderMainMenuToVirtualScreen(GameState* state)
{
    Vector2 virtualMouse = GetVirtualMousePosition(state);
    BeginTextureMode(state->Dungeon);
    ClearBackground(DARKGRAY);
    startButton.Draw(virtualMouse);
    quitButton.Draw(virtualMouse);
    EndTextureMode();
}
