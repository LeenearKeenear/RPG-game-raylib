#pragma once
#include "../lib/raylib/include/raylib.h"
#include "../lib/tileson/tileson.hpp"
#include "screen.h"
#include <string>
#include <vector>
#include <map>

<<<<<<< HEAD
// ================================================================
// Texture & Asset
// ================================================================

// dawg ini dipindah dawg 
// jumlah maksimum slot texture PNG yang bisa di-load
#define MAX_TEXTURES 4

// dawg ini dipindah dawg 
// enum buat milih slot texture — tambah di sini kalau ada asset baru
typedef enum
{
    TEXTURE_TILEMAP = 0,
    TEXTURE_KNIGHT,
    TEXTURE_SLIME,
    TEXTURE_SKELETON,
    TEXTURE_WOLF
} TextureAsset;

// dawg ini dipindah dawg 
extern Texture2D TexturesMap[MAX_TEXTURES];

=======
>>>>>>> e799af3f0a50656c282fcf81504179372f5fb8ac
// camera
extern Camera2D camera;

// ================================================================
<<<<<<< HEAD
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
    TILE_PLAYER_NEW,
    TILE_ENEMY_TEST
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
=======
>>>>>>> e799af3f0a50656c282fcf81504179372f5fb8ac
// Tileson Map System
// ================================================================

// representasi satu object dari object layer Tiled
// bisa berupa collision rect, spawn point, door, dll
struct MapObject
{
    std::string name;
    std::string type;
    std::string layerName;              // nama object layer asal di Tiled
    Rectangle bounds;
    std::vector<Vector2> polygonPoints; // titik polygon/polyline dalam world space
    bool hasPolygon = false;            // true kalau object ini punya polygon custom
    std::map<std::string, tson::Property> properties;
};

// info satu tileset — texture + metadata buat render
struct TilesetInfo
{
    Texture2D texture;
    int cols;
    int spacing;
    int firstgid;
    int lastgid; // firstgid tileset berikutnya - 1, dipake buat cari tileset yang bener pas render
};

// semua data map yang udah di-parse dari JSON Tiled
struct TilesonMapData
{
    int width;
    int height;
    int layerCount;
    int **tiles;
    std::vector<TilesetInfo> tilesets; // ganti single texture jadi vector
    std::vector<MapObject> Objects;
};

extern TilesonMapData *tilesonMap;

// hasil kalkulasi frustum: range index tile yang visible di layar
struct TileRange
{
    int minX; // kolom tile paling kiri yang visible
    int minY; // baris tile paling atas yang visible
    int maxX; // kolom tile paling kanan yang visible (exclusive)
    int maxY; // baris tile paling bawah yang visible (exclusive)
};

// data yang bisa dibaca oleh sistem debug
extern int lastTilesRendered;
extern TileRange currentVisibleRange;

// nama layer & object di Tiled — sesuaiin kalau beda
// #define COLLISION_LAYER_NAME "map_bound"
#define COLLISION_LAYER_NAME "obstacle" // penulisan define untuk layer
#define SPAWN_OBJECT_NAME "spawn" // penulisan define untuk object name
#define DOOR_TYPE_OBJECT_NAME "door" // penulisan define untuk type object name

// ================================================================
// Functions
// ================================================================

// load, render, unload map dari JSON Tiled
void LoadMap(const char *mapPath);
void RenderMap(void);
void UnloadMap(void);
void InitMap(void);

// query object dari object layer Tiled
std::vector<MapObject> TilesonGetObjectsByLayerName(const std::string &layerName);
std::vector<MapObject> TilesonGetObjectsByType(const std::string &type);
MapObject *TilesonGetObjectByName(const std::string &name);
