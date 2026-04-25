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
    TEXTURE_TILEMAP = 0,    ///< Tileset lingkungan utama
    TEXTURE_KNIGHT,         ///< Sprite pemain/karakter
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
 * @brief Tipe tile yang telah ditentukan sebelumnya untuk dunia game.
 */
typedef enum
{
    TILE_CLU_WALL, TILE_CMU_WALL, TILE_CRU_WALL,
    TILE_CML_WALL, TILE_M_WALL,   TILE_CMR_WALL,
    TILE_CLD_WALL, TILE_CMD_WALL, TILE_CRD_WALL,
    TILE_POOL,
    TILE_BIGMAN,
    TILE_GRASS1,
    TILE_GRASS2,
    TILE_DOOR_OPEN,
    TILE_DOOR_CLOSE,
    TILE_PLAYER_NEW
} TileType;

/**
 * @brief Definisi properti suatu tile.
 */
struct TileDefinition
{
    TileCoordinate CoordID; ///< Posisi pada tekstur tileset
    bool IsWalkable;        ///< Apakah entitas dapat melewati tile ini?
    bool HasInteraction;    ///< Apakah tile ini memicu suatu event?
};

/**
 * @brief Memuat tekstur ke dalam slot tertentu.
 */
void LoadTileTexture(TextureAsset Slot, const char *Path);

/**
 * @brief Me-render tile menggunakan TileType tertentu.
 */
void RenderTilePNG(int posX, int posY, TileType Type, float Rotation, TextureAsset Slot);

/**
 * @brief Helper untuk mendapatkan rectangle sumber dari koordinat grid.
 */
Rectangle GetFrame(int frameX, int frameY);