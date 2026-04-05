#pragma once
#include "../lib/raylib/include/raylib.h"

#define MAX_TEXTURES 3
typedef enum
{
    TEXTURE_TILEMAP = 0
} TextureAsset;

extern Texture2D TexturesMap[MAX_TEXTURES];
extern Camera2D camera;

// struct kordinat universal
typedef struct
{
    int x;
    int y;
} TileCoordinate;

// enum buat definisiin gambar biar enak
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
} TileType;

// struct buat koordinat tile
typedef struct
{
    TileCoordinate CoordinateTile;
    TileType type;
} sTile;

// sementara doang (ini buat entity (contoh player, enemy, npc))
typedef struct
{
    TileCoordinate PlayerPosition;
    float MoveTimer;
    float MoveDelay;
} Entity;

// struct buat tile properti
typedef struct
{
    TileCoordinate CoordID;
    bool IsWalkable;
    bool HasInteraction;
} TileDefinition;

// definisi struct
extern Entity Player;
extern sTile Door;
extern TileDefinition TileDefs[];

// ukuran tile buat di mapping sprite nya
#define TILE_WIDTH 32
#define TILE_HEIGHT 32
#define TILE_GAP 4

// enum buat texture id nya

void InitDrawMap(GameState *state);
void UpdatePlayer(GameState *state);
void RenderMap(GameState *state);