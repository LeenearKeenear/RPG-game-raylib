#include "../include/tiles.h"

Texture2D TexturesMap[MAX_TEXTURES];

void LoadTileTexture(TextureAsset Slot, const char *Path)
{
    Image img = LoadImage(Path);
    TexturesMap[Slot] = LoadTextureFromImage(img);
    UnloadImage(img);
}

void RenderTilePNG(int posX, int posY, TileType Type, float Rotation, TextureAsset Slot)
{
    TileDefinition TileProperty[] = {
        [TILE_CLU_WALL] = {{0, 0}, false, false},
        [TILE_CMU_WALL] = {{1, 0}, false, false},
        [TILE_CRU_WALL] = {{3, 0}, false, false},
        [TILE_CML_WALL] = {{0, 1}, false, false},
        [TILE_M_WALL] = {{1, 1}, false, false},
        [TILE_CMR_WALL] = {{3, 1}, false, false},
        [TILE_CLD_WALL] = {{0, 2}, false, false},
        [TILE_CMD_WALL] = {{1, 2}, false, false},
        [TILE_CRD_WALL] = {{3, 2}, false, false},
        [TILE_POOL] = {{12, 8}, false, false},
        [TILE_BIGMAN] = {{7, 0}, false, false},
        [TILE_GRASS1] = {{4, 4}, true, false},
        [TILE_GRASS2] = {{5, 4}, true, false},
        [TILE_DOOR_OPEN] = {{4, 2}, true, true},
        [TILE_DOOR_CLOSE] = {{5, 2}, false, true},
        [TILE_PLAYER_NEW] = {{3, 2}, false, false}};

    Rectangle Source = {
        (float)(TileProperty[Type].CoordID.x * (TILE_SIZE + TILE_GAP)),
        (float)(TileProperty[Type].CoordID.y * (TILE_SIZE + TILE_GAP)),
        (float)TILE_SIZE,
        (float)TILE_SIZE};

    Rectangle Destination = {(float)posX, (float)posY, (float)TILE_SIZE, (float)TILE_SIZE};
    Vector2 Origin = {0, 0};
    DrawTexturePro(TexturesMap[Slot], Source, Destination, Origin, Rotation, WHITE);
}

Rectangle GetFrame(int frameX, int frameY)
{
    return {
        (float)(frameX * (TILE_SIZE + TILE_GAP)),
        (float)(frameY * (TILE_SIZE + TILE_GAP)),
        (float)TILE_SIZE,
        (float)TILE_SIZE};
}
