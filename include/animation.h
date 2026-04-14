#pragma once
#include "../lib/raylib/include/raylib.h"
#include "../lib/tileson/tileson.hpp"
#include "screen.h"
#include <string>
#include <vector>
#include <map>
// ================================================================
// Animation System
// Handle semua animasi sprite untuk player, enemy, dan entity lain.
//
// Semua logic animasi (frame switching, timing, direction)
// dipusatin di sini biar gak nyebar ke mana-mana.
//
// TODO (pindahan dari map.h / map.cpp):
// - LoadTileTexture()(v)
// - RenderTilePNG()(v)
// - TileDefinition(v) struct
// - TileType(v) enum
// - TileCoordinate(v) struct
// - TextureAsset(v) enum
// - TexturesMap(v) array
// - MAX_TEXTURES(v) define
// - TILE_SIZE, TILE_GAP(v) definemk5                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         2 nfuyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy111111111111111111111111111111111111111111111111111111111111111111111111111111111112N,
//
// Setelah dipindah, update semua include dan pemanggilan
// di map.cpp, player.cpp, entities.cpp, dll.
// ================================================================

// jumlah maksimum slot texture PNG yang bisa di-load
#define MAX_TEXTURES 4

// enum buat milih slot texture — tambah di sini kalau ada asset baru
typedef enum
{
    TEXTURE_TILEMAP = 0,
    TEXTURE_KNIGHT,
    TEXTURE_SLIME,
    TEXTURE_SKELETON,
    TEXTURE_WOLF
} TextureAsset;


extern Texture2D TexturesMap[MAX_TEXTURES];

// koordinat universal buat posisi tile di spritesheet atau world
typedef struct
{
    int x;
    int y;
} TileCoordinate;

// enum semua jenis tile yang ada — tambah di sini kalau ada tile baru
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
    TILE_PLAYER_NEW,
    TILE_ENEMY_TEST
} TileType;

// properti tiap tile: posisi di spritesheet, bisa dilewatin, ada interaksi gak
typedef struct
{
    TileCoordinate CoordID;
    bool IsWalkable;
    bool HasInteraction;
} TileDefinition;

// ukuran tile dalam pixel + gap antar tile di spritesheet
#define TILE_SIZE 32
#define TILE_GAP 4

// load texture PNG ke slot yang ditentuin
void LoadTileTexture(TextureAsset Slot, const char *Path);

// render satu tile dari spritesheet ke posisi world
void RenderTilePNG(int pos_x, int pos_y, TileType Type, float Rotation, TextureAsset Slot);