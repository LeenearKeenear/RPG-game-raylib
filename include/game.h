#pragma once
#include "screen.h"
#include "render.h"
#include "debug.h"
#include "map.h"

class Game
{
public:
    void Open();
    void Loop();
    void Close();

private:
    Screen Screen;
    Render Render;
    Debug Debug;
    Map Map;
    void Update();
    void Draw();
};