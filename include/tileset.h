#pragma once
#include <raylib.h>

enum TileType
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
    COUNT
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
    void Load(const char *Path);
    void Unload();
    void Render(TileType TileType, TilePos TilePosition);
    TileDef &GetTileType(TileType TileType);

private:
    Texture2D Texture;
    TileDef TileDefs[COUNT];
    static const int TILE_SIZE = 32;
};