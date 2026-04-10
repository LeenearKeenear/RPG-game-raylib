#pragma once
#include <raylib.h>
#include "screen.h"

class Render
{
public:
    void Init();
    void Begin();
    void End();
    void Draw(const Screen &Screen);
    void Shutdown();

private:
    RenderTexture2D Dungeon;
};