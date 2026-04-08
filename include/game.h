#pragma once
#include "screen.h"
#include "render.h"
#include "debug.h"

class Game
{
public:
    void Init();
    void Run();
    void Shutdown();

private:
    Screen Screen;
    Render Render;
    Debug Debug;
    void Update();
    void Draw();
};