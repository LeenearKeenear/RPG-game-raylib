#pragma once
#include "../lib/raylib/include/raylib.h"
#include "../lib/tileson/tileson.hpp"
#include "screen.h"
#include <string>
#include <vector>
#include <map>

// ================================================================
// Texture & Asset
// ================================================================

// dawg ini dipindah dawg 
// jumlah maksimum slot texture PNG yang bisa di-load
#define MAX_TEXTURES 3

// dawg ini dipindah dawg 
// enum buat milih slot texture — tambah di sini kalau ada asset baru
typedef enum
{
    TEXTURE_TILEMAP = 0,
    TEXTURE_KNIGHT
} TextureAsset;

// dawg ini dipindah dawg 
extern Texture2D TexturesMap[MAX_TEXTURES];

// camera
extern Camera2D camera;

// ================================================================
// Tile System
// ================================================================

// dawg ini dipindah dawg 
// koordinat universal buat posisi tile di spritesheet atau world
typedef struct
{
    int x;
    int y;
} TileCoordinate;

// dawg ini dipindah dawg 
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
    TILE_PLAYER_NEW
} TileType;

// dawg ini dipindah dawg 
// properti tiap tile: posisi di spritesheet, bisa dilewatin, ada interaksi gak
typedef struct
{
    TileCoordinate CoordID;
    bool IsWalkable;
    bool HasInteraction;
} TileDefinition;

// dawg ini dipindah dawg 
// ukuran tile dalam pixel + gap antar tile di spritesheet
#define TILE_SIZE 32
#define TILE_GAP 4

// ================================================================
// Struct yang dihapus (map system lama, digantiin Tileson)
//
// sTile — tile dengan koordinat + type, digantiin TilesonMapData
// MapDataDefinition — struct map lama pake sTile**, digantiin TilesonMapData
// CurrentMap (extern MapDataDefinition*) — gak relevan setelah pindah ke Tileson
// ================================================================

// ================================================================
// Tileson Map System
// ================================================================

// representasi satu object dari object layer Tiled
// bisa berupa collision rect, spawn point, door, dll
struct MapObject
{
    std::string name;
    std::string type;
    Rectangle bounds;
    std::map<std::string, tson::Property> properties;
};

// semua data map yang udah di-parse dari JSON Tiled
struct TilesonMapData
{
    int width;          // lebar map dalam satuan tile
    int height;         // tinggi map dalam satuan tile
    int layerCount;     // jumlah tile layer
    int **tiles;        // array 2D tile per layer: tiles[layer][y*width+x]
    Texture2D tilesetTexture;
    int tilesetCols;        // jumlah kolom di spritesheet tileset
    int tilesetSpacing;     // gap antar tile di spritesheet
    int tilesetFirstgid;    // ID awal tile (biasanya 1)
    std::vector<MapObject> Objects; // semua object dari object layer
};

extern TilesonMapData *tilesonMap;

// ================================================================
// Functions
// ================================================================

// load texture PNG ke slot yang ditentuin
void LoadTileTexture(TextureAsset Slot, const char *Path);

// render satu tile dari spritesheet ke posisi world
void RenderTilePNG(int pos_x, int pos_y, TileType Type, float Rotation, TextureAsset Slot);

// load, render, unload map dari JSON Tiled
void LoadMap(const char *mapPath);
void RenderMap(void);
void UnloadMap(void);
void InitMap(void);

// query object dari object layer Tiled
std::vector<MapObject> TilesonGetObjectsByType(const std::string &type);
MapObject *TilesonGetObjectByName(const std::string &name);