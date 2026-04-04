#pragma once
#include "../lib/raylib/include/raylib.h"

#define MAX_TEXTURES 3
typedef enum
{
    TEXTURE_TILEMAP = 0
} TextureAsset;

extern Texture2D TexturesMap[MAX_TEXTURES];
extern Camera2D camera;

typedef struct
{
    int x;
    int y;
} sTile;

// sementara doang
typedef struct
{
    int x;
    int y;
} Entity;

extern Entity Player;

// ukuran tile buat di mapping sprite nya
#define TILE_WIDTH 32
#define TILE_HEIGHT 32
#define TILE_GAP 4


void InitDrawMap(GameState *state);
void UpdateMap(GameState *state);
void RenderMap(GameState *state);