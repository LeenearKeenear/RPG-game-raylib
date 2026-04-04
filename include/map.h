#pragma once
#include "../lib/raylib/include/raylib.h"

#define MAX_TEXTURES 1
typedef enum
{
    TEXTURE_TILEMAP = 0
} TextureAsset;

extern Texture2D TexturesMap[MAX_TEXTURES];

typedef struct
{
    int x;
    int y;
} sTile;

void InitDrawMap(GameState *state);
void UpdateMap(GameState *state);
void RenderMap(GameState *state);