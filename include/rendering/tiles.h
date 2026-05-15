#pragma once
#include "../lib/raylib/include/raylib.h"

constexpr int TILE_SIZE = 32;   ///< Ukuran standar tile dalam pixel
constexpr int TILE_GAP = 4;     ///< Jarak antar tile dalam tileset (jika ada)
constexpr int MAX_TEXTURES = 6; ///< Jumlah maksimum tekstur yang dimuat secara bersamaan

extern Texture2D TexturesMap[MAX_TEXTURES];

/**
 * @brief Pengidentifikasi (ID) untuk aset tekstur yang dimuat ke memori.
 * @note TEXTURE_TILEMAP digunakan oleh map renderer (map.cpp)
 */
enum TextureAsset
{
    TEXTURE_TILEMAP = 0, ///< Tileset untuk rendering map
    TEXTURE_KNIGHT,      ///< Sprite pemain/karakter
    TEXTURE_ITEMS,       ///< Ikon dan item koleksi
    TEXTURE_ENEMIES      ///< Sprite musuh
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

/**
 * @brief Helper untuk merender tile langsung ke layar tanpa mengakses TexturesMap secara manual.
 */
void DrawTileTexture(TextureAsset slot, int frameX, int frameY, Rectangle dest, Vector2 origin = {0, 0}, float rotation = 0.0f, Color tint = WHITE);