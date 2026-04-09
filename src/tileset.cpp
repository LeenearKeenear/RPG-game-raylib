#include "tileset.h"

void Tileset::Load(const char *Path)
{
    Texture = LoadTexture(Path);
    TileDefs[WALL_UP_LEFT] = {{0, 0}, true, false};
    TileDefs[WALL_UP_CENTER] = {{1, 0}, true, false};
    TileDefs[WALL_UP_RIGHT] = {{3, 0}, true, false};
    TileDefs[WALL_LEFT_CENTER] = {{0, 1}, true, false};
    TileDefs[WALL_RIGHT_CENTER] = {{3, 1}, true, false};
    TileDefs[WALL_DOWN_LEFT] = {{0, 2}, true, false};
    TileDefs[WALL_DOWN_RIGHT] = {{3, 2}, true, false};
    TileDefs[GROUND1] = {{1, 1}, false, false};
    TileDefs[GROUND2] = {{4, 4}, false, false};
    TileDefs[GRASS] = {{5, 4}, false, false};
    TileDefs[TREE1] = {{4, 5}, true, false};
    TileDefs[TREE2] = {{5, 5}, true, false};
    TileDefs[DOOR_OPEN] = {{4, 2}, false, true};
    TileDefs[DOOR_CLOSED] = {{5, 2}, false, true};
}

void Tileset::Unload()
{
    UnloadTexture(Texture);
}

void Tileset::Render(TileType TileType, TilePos TilePosition)
{
    Rectangle Source = {
        (float)(TileDefs[TileType].Position.x * (TILE_SIZE + 4)),
        (float)(TileDefs[TileType].Position.y * (TILE_SIZE + 4)),
        (float)TILE_SIZE,
        (float)TILE_SIZE
    };
    DrawTextureRec(Texture, Source, {(float)TilePosition.x, (float)TilePosition.y}, WHITE);
}

TileDef &Tileset::GetTileType(TileType TileType)
{
    return TileDefs[TileType];
}