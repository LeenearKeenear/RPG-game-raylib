#pragma once
#include "../lib/raylib/include/raylib.h"
#include "../lib/tileson/tileson.hpp"
#include "screen.h"

using TilesonMapData = struct
{
    int width;
    int height;
    int layerCount;
    int **tiles;
    Texture2D tilesetTexture;
    int tilesetCols;
    int tilesetSpacing;
    int tilesetFirstgid;
};

extern TilesonMapData *tilesonMap;

void TilesonLoadMap(const char *mapPath);
void TilesonUnloadMap();
void TilesonInit(GameState *state);
void TilesonRender(GameState *state);
void TilesonDebugDraw();
