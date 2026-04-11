#pragma once
#include "../lib/raylib/include/raylib.h"
#include "../lib/tileson/tileson.hpp"
#include "screen.h"
#include <string>
#include <vector>
#include <map>

// texture pack
// jumlah maksimum gambar texture png yang dibolehin
#define MAX_TEXTURES 3

// enum buat milih texture pack
typedef enum
{
    TEXTURE_TILEMAP = 0,
    TEXTURE_KNIGHT
} TextureAsset;

extern Texture2D TexturesMap[MAX_TEXTURES];
extern Camera2D camera;

// struct kordinat universal
typedef struct
{
    int x;
    int y;
} TileCoordinate;

// enum buat definisiin gambar biar enak
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

// struct buat koordinat tile
typedef struct
{
    TileCoordinate CoordinateTile;
    TileType type;
} sTile;

// struct buat tile properti
typedef struct
{
    TileCoordinate CoordID;
    bool IsWalkable;
    bool HasInteraction;
} TileDefinition;

// struct buat map
typedef struct
{
    int TileWidth;
    int TileHeight;
    sTile **Tiles;
    TileCoordinate SpawnPointPlayer;
} MapDataDefinition;

// definisi struct yang handle map
extern MapDataDefinition *CurrentMap;

// ukuran tile buat di mapping sprite nya (dalam bentuk pixel)
#define TILE_SIZE 32
#define TILE_GAP 4

// buat ngeload texture dari berbagai png
void LoadTileTexture(TextureAsset Slot, const char *Path);
// buat ngerender tile dari gambar png nya
void RenderTilePNG(int pos_x, int pos_y, TileType Type, float Rotation, TextureAsset Slot);

// buat nge handle map
struct MapObject
{
    std::string name;
    std::string type;
    Rectangle bounds;
    std::map<std::string, tson::Property> properties;
};


struct TilesonMapData
{
    int width;
    int height;
    int layerCount;
    int **tiles;
    Texture2D tilesetTexture;
    int tilesetCols;
    int tilesetSpacing;
    int tilesetFirstgid;
    std::vector<MapObject> Objects;
};

extern TilesonMapData *tilesonMap;

// query functions
std::vector<MapObject> TilesonGetObjectsByType(const std::string &type);
MapObject *TilesonGetObjectByName(const std::string &name);

void TilesonLoadMap(const char *mapPath);
void TilesonUnloadMap();
void TilesonInit(GameState *state);
void TilesonRender(GameState *state);
void TilesonDebugDraw();