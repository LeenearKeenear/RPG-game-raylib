#pragma once

// ================================================================
// Animation System
// Handle semua animasi sprite untuk player, enemy, dan entity lain.
//
// Semua logic animasi (frame switching, timing, direction)
// dipusatin di sini biar gak nyebar ke mana-mana.
//
// ================================================================

#include "raylib.h"

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

// load texture PNG ke slot yang ditentuin
void LoadTileTexture(TextureAsset Slot, const char *Path);

// render satu tile dari spritesheet ke posisi world
void RenderTilePNG(int pos_x, int pos_y, TileType Type, float Rotation, TextureAsset Slot);

// // --- ENUMS ---
// enum State
// {
//     IDLE,
//     WALK,
//     ATTACK,
//     DEAD
// };

// enum Direction
// {
//     LEFT,
//     RIGHT,
//     DOWN,
//     UP
// };

// // --- PLAYER STRUCT ---
// struct Player
// {
//     Vector2 position;

//     State state;
//     Direction direction;

//     int frame;
//     float frameTime;
//     float frameSpeed;

//     int walkFrameIndex;

//     bool isAttacking;
//     bool isDead;
// };

// // --- FUNCTION DECLARATIONS ---
// // Get frame rectangle from spritesheet
// Rectangle GetFrame(int frameX, int frameY);

// // Update player input and state
// void UpdatePlayer(Player &p);

// // Update animation frames based on state
// void UpdateAnimation(Player &p, float dt);

// // Draw player sprite
// void DrawPlayer(Player &p, Texture2D texture);