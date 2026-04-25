#pragma once
#include "raylib.h"

#define TILE_SIZE 32
#define TILE_GAP 4
#define MAX_TEXTURES 4

extern Texture2D TexturesMap[MAX_TEXTURES];

enum TextureAsset
{
    TEXTURE_TILEMAP = 0,
    TEXTURE_KNIGHT,
    TEXTURE_ITEMS,
    TEXTURE_ENEMIES
};

struct TileCoordinate
{
    int x;
    int y;
};

typedef enum
{
    TILE_CLU_WALL,
    TILE_CMU_WALL,
    TILE_CRU_WALL,
    TILE_CML_WALL,
    TILE_M_WALL,
    TILE_CMR_WALL,
    TILE_CLD_WALL,
    TILE_CMD_WALL,
    TILE_CRD_WALL,
    TILE_POOL,
    TILE_BIGMAN,
    TILE_GRASS1,
    TILE_GRASS2,
    TILE_DOOR_OPEN,
    TILE_DOOR_CLOSE,
    TILE_PLAYER_NEW
} TileType;

struct TileDefinition
{
    TileCoordinate CoordID;
    bool IsWalkable;
    bool HasInteraction;
};

void LoadTileTexture(TextureAsset Slot, const char *Path);
void RenderTilePNG(int posX, int posY, TileType Type, float Rotation, TextureAsset Slot);
Rectangle GetFrame(int frameX, int frameY);