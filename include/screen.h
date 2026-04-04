#pragma once
#include "../lib/raylib/include/raylib.h"

typedef struct {
    RenderTexture2D Dungeon;
    float ScaleMultiplier;
    int WindowScreenWidth;
    int WindowScreenHeight;
} GameState;

GameState InitScreen(void);
void UpdateGame(GameState *state);
void DrawRenderTexture(GameState *state);
void DrawRenderWindows(GameState *state);

