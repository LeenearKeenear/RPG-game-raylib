#include "../include/mainMenu.h"

static buttonTxt startButton;
static buttonTxt quitButton;

void InitMainMenu()
{
    int centerX = (GameScreenWidth / 2) - 50;
    int buttonSpacing = 50;
    int fontSize = 30;

    startButton = buttonTxt("Start Game", centerX, 300, fontSize, WHITE, 0.6F);
    quitButton = buttonTxt("Quit", centerX, 300 + buttonSpacing, fontSize, WHITE, 0.6F);
}

void UpdateMainMenu(GameState* state)
{
    Vector2 mousePosition = GetMousePosition();
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

void DrawMainMenu()
{
    BeginDrawing();
    ClearBackground(BLACK);
    startButton.Draw();
    quitButton.Draw();
    EndDrawing();
}
