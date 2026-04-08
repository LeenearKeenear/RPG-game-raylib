#pragma once
#include "screen.h"
#include "render.h"

class Game {
public:
    void Init();
    void Run();
    void Shutdown();

private:
    Screen Screen;
    Render Render;
    void Update();
    void Draw();
};