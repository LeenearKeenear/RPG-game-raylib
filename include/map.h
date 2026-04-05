#pragma once
#include "../lib/raylib/include/raylib.h"

#define MAX_TEXTURES 3
typedef enum
{
    TEXTURE_TILEMAP = 0
} TextureAsset;

extern Texture2D TexturesMap[MAX_TEXTURES];
extern Camera2D camera;

// struct kordinat universal
typedef struct
{
    int x;
    int y;
} TileCoordinate;

// TODO MULTI-MAP: nanti TileType bakal nambah banyak seiring
// nambahnya jenis tile baru. pastiin TileProperty[] di map.cpp ikut diupdate
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
} TileType;

// struct buat koordinat tile
typedef struct
{
    TileCoordinate CoordinateTile;
    TileType type;
} sTile;

// TODO MULTI-MAP: Entity nanti bakal butuh info
// di map mana dia berada (map id / pointer ke map aktif)
// sementara doang (ini buat entity (contoh player, enemy, npc))
typedef struct
{
    TileCoordinate PlayerPosition;
    float MoveTimer;
    float MoveDelay;
} Entity;

// struct buat tile properti
typedef struct
{
    TileCoordinate CoordID;
    bool IsWalkable;
    bool HasInteraction;
} TileDefinition;

// definisi struct
extern Entity Player;
extern sTile Door;
extern TileDefinition TileDefs[];

// ukuran tile buat di mapping sprite nya (dalam bentuk pixel)
#define TILE_WIDTH 32
#define TILE_HEIGHT 32
#define TILE_GAP 4

// TODO MULTI-MAP: WORLD_WIDTH dan WORLD_HEIGHT harus jadi bagian dari
// struct MapData, bukan variabel global. tiap map punya ukuran sendiri
// ukuran worldnya (sementara 1 world dulu)
extern const int WORLD_WIDTH;
extern const int WORLD_HEIGHT;

// TODO MULTI-MAP: tambah fungsi LoadMap(const char* file) dan UnloadMap()
// buat swap antar map
void InitDrawMap(GameState *state);
void UpdatePlayer(GameState *state);
void RenderMap(GameState *state);