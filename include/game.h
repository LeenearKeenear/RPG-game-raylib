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
    Camera2D Camera;
    float CameraZoom = 1.0f;
    Rectangle DeadZone;
    void Update();
    void Draw();
};