#include "../include/tiles.h"

Texture2D TexturesMap[MAX_TEXTURES];

/**
 * Memuat tekstur dari penyimpanan (disk) ke slot tekstur GPU tertentu.
 * Memastikan data gambar di CPU dihapus setelah ditransfer ke GPU.
 */
void LoadTileTexture(TextureAsset Slot, const char *Path)
{
    Image img = LoadImage(Path);
    TexturesMap[Slot] = LoadTextureFromImage(img);
    UnloadImage(img);
}

/**
 * Fungsi pembantu untuk menghitung rectangle sumber dari koordinat berbasis grid.
 * Berguna untuk animasi dan perenderan tile secara manual.
 */
Rectangle GetFrame(int frameX, int frameY)
{
    return {
        (float)(frameX * (TILE_SIZE + TILE_GAP)),
        (float)(frameY * (TILE_SIZE + TILE_GAP)),
        (float)TILE_SIZE,
        (float)TILE_SIZE};
}

/**
 * Merender tile ke layar.
 */
void DrawTileTexture(TextureAsset slot, int frameX, int frameY, Rectangle dest, Vector2 origin, float rotation, Color tint)
{
    Rectangle src = GetFrame(frameX, frameY);
    DrawTexturePro(TexturesMap[slot], src, dest, origin, rotation, tint);
}
