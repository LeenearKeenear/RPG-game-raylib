#pragma once
#include "raylib.h"

#define TILE_SIZE 32        ///< Ukuran standar tile dalam pixel
#define TILE_GAP 4         ///< Jarak antar tile dalam tileset (jika ada)
#define MAX_TEXTURES 4      ///< Jumlah maksimum tekstur yang dimuat secara bersamaan

extern Texture2D TexturesMap[MAX_TEXTURES];

/**
 * @brief Pengidentifikasi (ID) untuk aset tekstur yang dimuat ke memori.
 */
enum TextureAsset
{
    TEXTURE_KNIGHT = 0,         ///< Sprite pemain/karakter
    TEXTURE_ITEMS,          ///< Ikon dan item koleksi
    TEXTURE_ENEMIES         ///< Sprite musuh
};

/**
 * @brief Koordinat dalam sistem grid.
 */
struct TileCoordinate
{
    int x;
    int y;
};

/**
 * @brief Memuat tekstur ke dalam slot tertentu.
 */
void LoadTileTexture(TextureAsset Slot, const char *Path);

/**
 * @brief Helper untuk mendapatkan rectangle sumber dari koordinat grid.
 */
Rectangle GetFrame(int frameX, int frameY);