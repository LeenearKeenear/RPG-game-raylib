#pragma once
#include "tileset.h"

class Map
{
public:
    void Init();
    void Render();

private:
    static const int MAP_WIDTH = 20;
    static const int MAP_HEIGHT = 12;
    static const int TILE_SIZE = 32;
    TileType Tiles[MAP_HEIGHT][MAP_WIDTH];
    Tileset Tileset;
};