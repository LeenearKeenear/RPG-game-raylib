#include "../include/map.h"

void Map::Init()
{
    Tileset.Load("texture/map.png");
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            Tiles[y][x] = GROUND2;
        }
    }
}

void Map::Render()
{
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            Tileset.Render(Tiles[y][x], {x * TILE_SIZE, y * TILE_SIZE});
        }
    }
}