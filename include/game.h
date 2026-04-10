#pragma once
#include "screen.h"
#include "render.h"
#include "debug.h"
#include "map.h"
#include "player.h"
#include "tileset.h"

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
    Tileset Tileset;
    Map Map;
    Player Player;
    void Update();
    void Draw();
};