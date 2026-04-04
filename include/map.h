#pragma once
#include "../lib/raylib/include/raylib.h"

#define MAX_TEXTURES 3
typedef enum
{
    TEXTURE_TILEMAP = 0
} TextureAsset;

extern Texture2D TexturesMap[MAX_TEXTURES];
extern Camera2D camera;

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

typedef struct
{
    int x;
    int y;
    TileType type;
} sTile;

// sementara doang (ini buat entity (contoh player, enemy, npc))
typedef struct
{
    int x;
    int y;
    float MoveTimer;
    float MoveDelay;
} Entity;

// sementara. ini buat prop atributte (contoh. wall, door, chest etc)
typedef struct 
{
    int x;
    int y;
} PropsAttributes;


extern Entity Player;
extern PropsAttributes Door;

// ukuran tile buat di mapping sprite nya
#define TILE_WIDTH 32
#define TILE_HEIGHT 32
#define TILE_GAP 4

// enum buat texture id nya
typedef struct
{
    int x;
    int y;
} TileCoordinate;



extern TileCoordinate TileCoords[];


void InitDrawMap(GameState *state);
void UpdatePlayer(GameState *state);
void RenderMap(GameState *state);