#pragma once

// ================================================================
// Animation System
// Handle semua animasi sprite untuk player, enemy, dan entity lain.
//
// Semua logic animasi (frame switching, timing, direction)
// dipusatin di sini biar gak nyebar ke mana-mana.
//
// Module ini juga contain tile rendering system yang dipakai
// buat render spritesheet-based tiles dan character sprites.
// ================================================================

#include "raylib.h"

// ================================================================
// Texture & Asset
// ================================================================

// jumlah maksimum slot texture PNG yang bisa di-load
#define MAX_TEXTURES 3

// enum buat milih slot texture — tambah di sini kalau ada asset baru
typedef enum
{
    TEXTURE_TILEMAP = 0,
    TEXTURE_KNIGHT,
    TEXTURE_ITEMS // test menambahkan texture untuk player slot
} TextureAsset;

// global texture array — diakses dari file lain via extern
extern Texture2D TexturesMap[MAX_TEXTURES];

// ================================================================
// Tile System
// ================================================================

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
    TILE_PLAYER_NEW
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

// ================================================================
// Tile Rendering Functions
// ================================================================

// load texture PNG ke slot yang ditentuin
void LoadTileTexture(TextureAsset Slot, const char *Path);

// render satu tile dari spritesheet ke posisi world
void RenderTilePNG(int pos_x, int pos_y, TileType Type, float Rotation, TextureAsset Slot);

// ambil source rectangle dari spritesheet berdasarkan frame koordinat
Rectangle GetFrame(int frameX, int frameY);

// ================================================================
// Animation State & Direction
// ================================================================

enum State
{
    IDLE,
    WALK,
    ATTACK,
    DEAD
};

enum Direction
{
    LEFT,
    RIGHT,
    DOWN,
    UP
};

// ================================================================
// AnimationPlayer Struct
// Data animasi untuk satu entity (player, enemy, dll)
// ================================================================
struct AnimationPlayer {
    Vector2 position;

    State state;
    Direction direction;

    int frame;
    float frameTime;
    float frameSpeed;

    int walkFrameIndex;

    bool isAttacking;
    bool isDead;
};

// ================================================================
// Animation Functions
// ================================================================

// state setters — set direction dan state animasi
void UpdatePlayerWalkUp(AnimationPlayer &p);
void UpdatePlayerWalkDown(AnimationPlayer &p);
void UpdatePlayerWalkLeft(AnimationPlayer &p);
void UpdatePlayerWalkRight(AnimationPlayer &p);
void UpdatePlayerIdle(AnimationPlayer &p);
void UpdatePlayerAttack(AnimationPlayer &p);
void UpdatePlayerDeath(AnimationPlayer &p);

// update frame animasi berdasarkan delta time
void UpdateAnimation(AnimationPlayer &p, float dt);

// render player sprite ke layar
void DrawPlayer(AnimationPlayer &p);