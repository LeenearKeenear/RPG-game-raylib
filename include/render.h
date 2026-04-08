#pragma once
#include "../lib/raylib/include/raylib.h"
#include "screen.h"

class Render {
public:
    void Init();
    void TextureMode(const Screen &Screen);
    void Drawing(const Screen &Screen);
    void Shutdown();

private:
    RenderTexture2D Dungeon;
    void DebugMouse(const Screen &Screen);
    // void DebugMenu(const Screen &Screen);
    // void DebugPlayer(const Screen &Screen);
};