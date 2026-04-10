#include "tileset.h"

void Tileset::LoadMap(const char *Path)
{
    MapTexture = LoadTexture(Path);

    TileMapDefs[WALL_UP_LEFT] = {{0, 0}, true, false};
    TileMapDefs[WALL_UP_CENTER] = {{1, 0}, true, false};
    TileMapDefs[WALL_UP_RIGHT] = {{3, 0}, true, false};
    TileMapDefs[WALL_LEFT_CENTER] = {{0, 1}, true, false};
    TileMapDefs[WALL_RIGHT_CENTER] = {{3, 1}, true, false};
    TileMapDefs[WALL_DOWN_LEFT] = {{0, 2}, true, false};
    TileMapDefs[WALL_DOWN_RIGHT] = {{3, 2}, true, false};
    TileMapDefs[GROUND1] = {{1, 1}, false, false};
    TileMapDefs[GROUND2] = {{4, 4}, false, false};
    TileMapDefs[GRASS] = {{5, 4}, false, false};
    TileMapDefs[TREE1] = {{4, 5}, true, false};
    TileMapDefs[TREE2] = {{5, 5}, true, false};
    TileMapDefs[DOOR_OPEN] = {{4, 2}, false, true};
    TileMapDefs[DOOR_CLOSED] = {{5, 2}, false, true};
}

void Tileset::LoadChar(const char *Path)
{
    CharTexture = LoadTexture(Path);

    TileCharDefs[CHARACTER1]  = {{4, 0}, false, false};
    TileCharDefs[CHARACTER2] = {{10, 0}, false, false};
}

void Tileset::UnloadAll()
{
    UnloadTexture(MapTexture);
    UnloadTexture(CharTexture);
}

void Tileset::RenderMap(int TileType, TilePos TilePosition)
{
    Rectangle Source = {
        (float)(TileMapDefs[TileType].Position.x * (TILE_SIZE + 4)),
        (float)(TileMapDefs[TileType].Position.y * (TILE_SIZE + 4)),
        (float)TILE_SIZE,
        (float)TILE_SIZE
    };
    DrawTextureRec(MapTexture, Source, {(float)TilePosition.x, (float)TilePosition.y}, WHITE);
}

void Tileset::RenderChar(int TileType, TilePos TilePosition)
{
    Rectangle Source = {
        (float)(TileCharDefs[TileType].Position.x * (TILE_SIZE + 4)),
        (float)(TileCharDefs[TileType].Position.y * (TILE_SIZE + 4)),
        (float)TILE_SIZE,
        (float)TILE_SIZE
    };
    DrawTextureRec(CharTexture, Source, {(float)TilePosition.x, (float)TilePosition.y}, WHITE);
}