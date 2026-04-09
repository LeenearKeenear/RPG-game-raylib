#pragma once
#include "../lib/raylib/include/raylib.h"
#include "../lib/tileson/tileson.hpp"
#include "screen.h"

#define TILE_SIZE 16

using TilesonMapData = struct
{
    int width;
    int height;
    int *tiles;
    Texture2D tilesetTexture;
};

extern TilesonMapData *tilesonMap;

void TilesonLoadMap(const char *mapPath);
void TilesonUnloadMap();
void TilesonInit(GameState *state);
void TilesonRender(GameState *state);
void TilesonDebugDraw();
