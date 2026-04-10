#pragma once
#include <raylib.h>

enum TileMapType
{
    WALL_UP_LEFT,
    WALL_UP_CENTER,
    WALL_UP_RIGHT,
    WALL_LEFT_CENTER,
    WALL_RIGHT_CENTER,
    WALL_DOWN_LEFT,
    WALL_DOWN_RIGHT,
    GROUND1,
    GROUND2,
    GRASS,
    TREE1,
    TREE2,
    DOOR_OPEN,
    DOOR_CLOSED,
    TILE_MAP
};

enum TileCharType
{
    CHARACTER1,
    CHARACTER2,
    TILE_CHAR
};

struct TilePos
{
    int x;
    int y;
};

struct TileDef
{
    TilePos Position;
    bool Blocked;
    bool Interaction;
};

class Tileset
{
public:
    void LoadMap(const char *Path);
    void LoadChar(const char *Path);
    void UnloadAll();
    void RenderMap(int TileType, TilePos TilePosition);
    void RenderChar(int TileType, TilePos TilePosition);
    TileDef &GetTileMapType(TileMapType TileMapType) { return TileMapDefs[TileMapType]; }
    TileDef &GetTileCharType(TileCharType TileCharType) { return TileCharDefs[TileCharType]; }

private:
    Texture2D MapTexture;
    Texture2D CharTexture;
    TileDef TileMapDefs[TILE_MAP];
    TileDef TileCharDefs[TILE_CHAR];
    static const int TILE_SIZE = 32;
};