// #pragma once
// #include "../lib/raylib/include/raylib.h"

// #define MAX_TEXTURES 3
// typedef enum
// {
//     TEXTURE_TILEMAP = 0
// } TextureAsset;

// extern Texture2D TexturesMap[MAX_TEXTURES];
// extern Camera2D camera;

// // struct kordinat universal
// typedef struct
// {
//     int x;
//     int y;
// } TileCoordinate;

// // TODO MULTI-MAP: nanti TileType bakal nambah banyak seiring
// // nambahnya jenis tile baru. pastiin TileProperty[] di map.cpp ikut diupdate
// // enum buat definisiin gambar biar enak
// typedef enum
// {
//     TILE_CLU_WALL,
//     TILE_CMU_WALL,
//     TILE_CRU_WALL,
//     TILE_CML_WALL,
//     TILE_M_WALL,
//     TILE_CMR_WALL,
//     TILE_CLD_WALL,
//     TILE_CMD_WALL,
//     TILE_CRD_WALL,
//     TILE_POOL,
//     TILE_BIGMAN,
//     TILE_GRASS1,
//     TILE_GRASS2,
//     TILE_DOOR_OPEN,
//     TILE_DOOR_CLOSE,
// } TileType;

// // struct buat koordinat tile
// typedef struct
// {
//     TileCoordinate CoordinateTile;
//     TileType type;
// } sTile;

// // TODO MULTI-MAP: Entity nanti bakal butuh info
// // di map mana dia berada (map id / pointer ke map aktif)
// // sementara doang (ini buat entity (contoh player, enemy, npc))
// typedef struct
// {
//     TileCoordinate PlayerPosition;
//     float MoveTimer;
//     float MoveDelay;
// } Entity;

// // struct buat tile properti
// typedef struct
// {
//     TileCoordinate CoordID;
//     bool IsWalkable;
//     bool HasInteraction;
// } TileDefinition;

// // definisi struct
// extern Entity Player;
// extern sTile Door;
// extern TileDefinition TileDefs[];

// // ukuran tile buat di mapping sprite nya (dalam bentuk pixel)
// #define TILE_WIDTH 32
// #define TILE_HEIGHT 32
// #define TILE_GAP 4

// // TODO MULTI-MAP: WORLD_WIDTH dan WORLD_HEIGHT harus jadi bagian dari
// // struct MapData, bukan variabel global. tiap map punya ukuran sendiri
// // ukuran worldnya (sementara 1 world dulu)
// extern const int WORLD_WIDTH;
// extern const int WORLD_HEIGHT;

// // TODO MULTI-MAP: tambah fungsi LoadMap(const char* file) dan UnloadMap()
// // buat swap antar map
// void InitDrawMap(GameState *state);
// void UpdatePlayer(GameState *state);
// void RenderMap(GameState *state);

#pragma once
#include "../lib/raylib/include/raylib.h"

enum Visual
{
    WALL_TOP_LEFT,
    WALL_TOP_RIGHT,
    WALL_BOTTOM_LEFT,
    WALL_BOTTOM_RIGHT,
    WALL_TOP,
    WALL_LEFT,
    WALL_RIGHT,
    FLOOR,
    TREE
};

enum Type
{
    OUTSIDE,
    SURFACE,
    BLOCK
};

struct Tile
{
    Visual Visual;
    Type Type;
};

struct Map
{
    static const int TILE_SIZE = 32;
    static const int MAP_WIDTH = 30;
    static const int MAP_HEIGHT = 20;

    Tile Tiles[MAP_HEIGHT][MAP_WIDTH];

    // inisialisasi tile dengan contoh map sederhana
    void Init()
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            for (int x = 0; x < MAP_WIDTH; x++)
            {
                // jika tile berada di tepi, maka visualnya adalah wall
                if (x == 0 && y == 0)
                {
                    Tiles[y][x] = {WALL_TOP_LEFT, BLOCK};
                }
                else if (x == MAP_WIDTH - 1 && y == 0)
                {
                    Tiles[y][x] = {WALL_TOP_RIGHT, BLOCK};
                }
                else if (x == 0 && y == MAP_HEIGHT - 1)
                {
                    Tiles[y][x] = {WALL_BOTTOM_LEFT, BLOCK};
                }
                else if (x == MAP_WIDTH - 1 && y == MAP_HEIGHT - 1)
                {
                    Tiles[y][x] = {WALL_BOTTOM_RIGHT, BLOCK};
                }
                else if (y == 0)
                {
                    Tiles[y][x] = {WALL_TOP, BLOCK};
                }
                else if (y == MAP_HEIGHT - 1)
                {
                    Tiles[y][x] = {WALL_TOP, BLOCK};
                }
                else if (x == 0)
                {
                    Tiles[y][x] = {WALL_LEFT, BLOCK};
                }
                else if (x == MAP_WIDTH - 1)
                {
                    Tiles[y][x] = {WALL_RIGHT, BLOCK};
                }
                else // jika tile bukan di tepi, maka visualnya floor
                {
                    Tiles[y][x] = {FLOOR, SURFACE};
                }
            }
        }
    }

    // cek apakah tile di koordinat yang dikirimkan bertipe blocked atau tidak
    bool IsBlocked(int x, int y)
    {
        // apakah posisi berada di luar map atau tidak
        if (x < 0 || y < 0 || x > MAP_WIDTH - 1 || y > MAP_HEIGHT - 1)
            return true;

        // cek tipe tile di posisi koordinat yang dikirimkan
        return Tiles[y][x].Type == BLOCK;
    }

    // pilih visual apa yang ditampilkan di posisi tile tertentu
    Rectangle GetSource(Visual Visual)
    {
        int TileX = 0;
        int TileY = 0;

        switch (Visual)
        {
        case WALL_TOP_LEFT:
            TileX = 0;
            TileY = 0;
            break;
        case WALL_TOP_RIGHT:
            TileX = 3;
            TileY = 0;
            break;
        case WALL_BOTTOM_LEFT:
            TileX = 0;
            TileY = 2;
            break;
        case WALL_BOTTOM_RIGHT:
            TileX = 3;
            TileY = 2;
            break;
        case WALL_TOP:
            TileX = 1;
            TileY = 0;
            break;
        case WALL_LEFT:
            TileX = 0;
            TileY = 1;
            break;
        case WALL_RIGHT:
            TileX = 3;
            TileY = 1;
            break;
        case FLOOR:
            TileX = 4;
            TileY = 4;
            break;
        case TREE:
            TileX = 5;
            TileY = 5;
            break;
        }

        // hitung posisi visual tile pada gambar beserta ukuran gap nya
        return {TileX * (TILE_SIZE + 4), TileY * (TILE_SIZE + 4), 32, 32};
    }

    // menggambar map ke render texture
    void Draw(Texture2D TileMap)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            for (int x = 0; x < MAP_WIDTH; x++)
            {
                Tile Tile = Tiles[y][x];

                Rectangle Source = GetSource(Tile.Visual);
                Vector2 Position = {
                    x * (float)TILE_SIZE,
                    y * (float)TILE_SIZE
                };

                // mirip draw texture pro, hanya saja lebih sederhana
                DrawTextureRec(TileMap, Source, Position, WHITE);
            }
        }
    }
};