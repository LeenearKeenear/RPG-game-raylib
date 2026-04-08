#pragma once
#include "../lib/raylib/include/raylib.h"

class Screen
{
public:
    static const int GAME_WIDTH = 1280;
    static const int GAME_HEIGHT = 720;
    void Init();
    void Update();
    void Shutdown();
    float GetGameScale() const;
    int GetWindowWidth() const;
    int GetWindowHeight() const;

private:
    const float SCALE_MULTIPLIER = 0.7f;
    const float SCALE_MIN = 0.4f;
    float GameScale = 1.0f;
    int WindowWidth = 0;
    int WindowHeight = 0;
};