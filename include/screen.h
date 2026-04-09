#pragma once
#include "../lib/raylib/include/raylib.h"

// definisi size render virtualnya
extern const int GameScreenWidth;
extern const int GameScreenHeight;

// definisi gamestate
typedef enum {
    MAIN_MENU,
    PLAY
} ScreenState;
typedef struct {
    RenderTexture2D Dungeon;
    float ScaleMultiplier;
    int WindowScreenWidth;
    int WindowScreenHeight;
    ScreenState currentScreen;
} GameState;

GameState InitScreen(void);
void UpdateGame(GameState *state);
void InitAll(void);
void DrawRenderTexture(GameState *state);
void DrawRenderWindows(GameState *state);
void GameShutDown(GameState *state);

