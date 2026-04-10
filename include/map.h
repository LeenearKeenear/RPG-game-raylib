#pragma once
#include "tileset.h"

class Map
{
public:
    void Init(Tileset *Tileset);
    void Render();
    Tileset *GetTilesetRef() { return TilesetRef; }
    int GetTileSize() const { return TILE_SIZE; }
    int GetWidth() { return MAP_WIDTH; }
    int GetHeight() { return MAP_HEIGHT; }

private:
    static const int MAP_WIDTH = 50;
    static const int MAP_HEIGHT = 50;
    static const int TILE_SIZE = 32;
    TileMapType Tiles[MAP_HEIGHT][MAP_WIDTH];
    Tileset *TilesetRef;
    friend class Player;
};