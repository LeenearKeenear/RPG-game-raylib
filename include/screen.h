#pragma once
#include "../raylib/include/raylib.h"

// ======================
// CONFIG
// ======================
namespace Config
{
    const int GAME_WIDTH = 1280;
    const int GAME_HEIGHT = 720;
}

// ======================
// GAME STATE
// ======================
struct GameState
{
    RenderTexture2D Dungeon;
    float ScaleMultiplier;
    int WindowWidth;
    int WindowHeight;
};

// ======================
// FUNCTIONS
// ======================
GameState InitScreen();
void UpdateGameState(GameState *Game);
void DrawRenderTexture(GameState *Game);
void DrawRenderWindow(GameState *Game);