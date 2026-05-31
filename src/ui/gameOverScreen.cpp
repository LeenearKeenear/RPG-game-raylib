#include "gameOverScreen.h"
#include "player.h"
#include "item.h"
#include "inventory.h"
#include "../lib/raylib/include/raylib.h"
#include "button.h"

static Texture2D goTitle = {0};
static Texture2D goRevive = {0};
static buttonImage goToMain;
static float goTitleY, goReviveY, goToMainY;
static bool goLoaded = false;

void InitGameOverScreen()
{
    if (goLoaded) return;

    Image img = LoadImage("assets/textures/gameOver/gameover.png");
    goTitle = LoadTextureFromImage(img);
    UnloadImage(img);

    img = LoadImage("assets/textures/gameOver/gameover-revive.png");
    goRevive = LoadTextureFromImage(img);
    UnloadImage(img);

    goTitleY = (GameScreenHeight - goTitle.height) / 2.0F - 80;
    goReviveY = goTitleY + goTitle.height + 10;
    goToMainY = goReviveY + goRevive.height + 85;

    Vector2 toMainPos = {GameScreenWidth / 2.0F, goToMainY};
    goToMain = buttonImage("assets/textures/gameOver/gameover-to-main.png", toMainPos, 1.0F, 0.6F);

    goLoaded = true;
}

void UpdateGameOverScreen(GameState *state)
{
    if (IsKeyPressed(KEY_R))
    {
        PlayerInstance.Anim.isDead = false;
        PlayerInstance.Anim.isAttacking = false;
        PlayAnimation(PlayerInstance.Anim, IDLE, PlayerInstance.Anim.direction);
        PlayerInstance.Health = PlayerInstance.MaxHealth;
        PlayerInstance.Mana = PlayerInstance.MaxMana;
        PlayerInstance.KnockbackVelocity = {0, 0};
        state->currentScreen = PLAY;
        return;
    }

    Vector2 mousePos = GetVirtualMousePosition(state);
    if (goToMain.isClicked(mousePos, IsMouseButtonPressed(MOUSE_LEFT_BUTTON)))
    {
        state->enteredLoading = false;
        state->loadingStage = 0;
        state->loadingProgress = 0.0F;
        state->loadingComplete = false;
        state->currentScreen = MAIN_MENU;
    }
}

void RenderGameOverScreen(GameState *state)
{
    if (!goLoaded) InitGameOverScreen();

    DrawRenderTexture(state);

    BeginTextureMode(state->Dungeon);

    DrawRectangle(0, 0, GameScreenWidth, GameScreenHeight, {0, 0, 0, 102});

    DrawTexture(goTitle, (GameScreenWidth - goTitle.width) / 2, (int)goTitleY, WHITE);

    DrawTexture(goRevive, (GameScreenWidth - goRevive.width) / 2, (int)goReviveY, WHITE);

    const char *hint = "Press 'R'";
    int hintSize = 20;
    int hintW = MeasureText(hint, hintSize);
    DrawText(hint, (GameScreenWidth - hintW) / 2, (int)goReviveY + goRevive.height + 10, hintSize, GRAY);

    Vector2 mousePos = GetVirtualMousePosition(state);
    goToMain.Draw(mousePos);

    EndTextureMode();
}
