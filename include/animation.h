#pragma once

/**
 * @file animation.h
 * @brief Animation System & Tile Rendering Module
 *
 * Handle semua animasi sprite untuk player, enemy, dan entity lain.
 * Semua logic animasi (frame switching, timing, direction) dipusatin di sini.
 * Module ini juga contain tile rendering system buat render spritesheet-based tiles.
 */

#include "raylib.h"

/*==============================================================================
 * Texture & Asset Management
 *==============================================================================*/

/** Jumlah maksimum slot texture PNG yang bisa di-load */
#define MAX_TEXTURES 6

/**
 * @brief Enum buat milih slot texture
 * @note TEXTURE_KNIGHT nanti bakal direfaktor jadi lebih general
 */
typedef enum
{
    TEXTURE_TILEMAP = 0,
    TEXTURE_KNIGHT,
    TEXTURE_ITEMS,   // test menambahkan texture untuk player slot
    TEXTURE_ENEMIES, 
} TextureAsset;

/** Global texture array - diakses dari file lain pake extern */
extern Texture2D TexturesMap[MAX_TEXTURES];

/*==============================================================================
 * Tile System
 *==============================================================================*/

/**
 * @brief Koordinat universal buat posisi tile di spritesheet atau world
 */
typedef struct
{
    int x; /**< Koordinat X */
    int y; /**< Koordinat Y */
} TileCoordinate;

/**
 * @brief Semua jenis tile yang ada
 * @note keknya ini gak kepake
 */
typedef enum
{
    TILE_PLAYER_NEW,
    TILE_CHEST_OPEN,
    TILE_CHEST_CLOSED,   /**< @deprecated Cuma placeholder, gak dipake */
    TILE_ENEMY_SLIME,    /**< Slime */
    TILE_ENEMY_SKELETON, /**< Skeleton */
    TILE_ENEMY_WOLF,     /**< Wolf */
    TILE_ITEM_POTION,
    TILE_WEAPON
} TileType;

/**
 * @brief Properti tiap tile
 * @note gak kepake
 */
typedef struct
{
    TileCoordinate CoordID; /**< Posisi tile di spritesheet */
    bool IsWalkable;        /**< True kalo player/enemy bisa lewat */
    bool HasInteraction;    /**< True kalo tile punya event interaksi */
} TileDefinition;

/** Ukuran tile dalam pixel */
#define TILE_SIZE 32

/** Jarak antar tile di spritesheet (padding) */
#define TILE_GAP 4

/*==============================================================================
 * Tile Rendering Functions
 *==============================================================================*/

/**
 * @brief Load texture PNG ke slot yang ditentuin
 * @param Slot Slot texture (dari enum TextureAsset)
 * @param Path File path ke gambar PNG
 */
void LoadTileTexture(TextureAsset Slot, const char *Path);

/**
 * @brief Render satu tile dari spritesheet ke posisi world
 * @param pos_x Posisi world X (pixel)
 * @param pos_y Posisi world Y (pixel)
 * @param Type Jenis tile (dari enum TileType)
 * @param Rotation Rotasi dalam derajat
 * @param Slot Texture asset yang dipake
 */
void RenderTilePNG(int pos_x, int pos_y, TileType Type, float Rotation, TextureAsset Slot);

/**
 * @brief Render satu tile dari spritesheet ke posisi world dengan ukuran kecil
 * @param Slot Texture asset yang dipake
 * @param sheetCoord Koordinat dalam png texture
 * @param worldPos Posisi dalam world
 * @param scale Skala pengecilan untuk texture
 * @param
 */
void DrawSmallSprite(TextureAsset slot, Vector2 sheetCoord, Vector2 worldPos, float scale);

/**
 * @brief Ambil source rectangle dari spritesheet berdasarkan frame koordinat
 * @param frameX Koordinat X frame (dalam satuan tile)
 * @param frameY Koordinat Y frame (dalam satuan tile)
 * @return Rectangle buat raylib DrawTextureRec()
 */
Rectangle GetFrame(int frameX, int frameY);

/*==============================================================================
 * Animation State & Direction
 *==============================================================================*/

/** State animasi buat entity (player, enemy, dll) */
enum State
{
    IDLE,   /**< Diam/standby */
    WALK,   /**< Jalan */
    ATTACK, /**< Nyerrang */
    DEAD    /**< Mati */
};

/** Arah hadap entity buat nentuin sprite mana yang dipake */
enum Direction
{
    LEFT,  /**< Ngadep kiri */
    RIGHT, /**< Ngadep kanan */
    DOWN,  /**< Ngadep bawah */
    UP     /**< Ngadep atas */
};

/*==============================================================================
 * AnimationPlayer Struct
 *==============================================================================*/

/**
 * @brief Data animasi buat satu entity (player, enemy, dll)
 */
struct AnimationPlayer
{
    Vector2 position; /**< Posisi entity di world (pixel) */

    State state;         /**< State animasi saat ini (IDLE/WALK/ATTACK/DEAD) */
    Direction direction; /**< Arah hadap entity saat ini */

    int frame;        /**< Index frame animasi yang ditampilin (wrapper dari walkFrameIndex) */
    float frameTime;  /**< Akumulator waktu buat timing animasi (dalam detik) */
    float frameSpeed; /**< Kecepatan ganti frame (durasi per frame, misal 0.1 = 10 FPS) */

    int walkFrameIndex; /**< Index frame buat mapping gambar dari texture pack */

    bool isAttacking; /**< Flag ngecek apakah entity sedang attack */
    bool isDead;      /**< Flag ngecek apakah entity udah mati */
};

/*==============================================================================
 * Animation Functions
 *==============================================================================*/

/** @name State Setters
 *  @brief Set direction dan state animasi
 *  @{
 */
void UpdatePlayerWalkUp(AnimationPlayer &p);    /**< Set animasi jalan ke atas */
void UpdatePlayerWalkDown(AnimationPlayer &p);  /**< Set animasi jalan ke bawah */
void UpdatePlayerWalkLeft(AnimationPlayer &p);  /**< Set animasi jalan ke kiri */
void UpdatePlayerWalkRight(AnimationPlayer &p); /**< Set animasi jalan ke kanan */
void UpdatePlayerIdle(AnimationPlayer &p);      /**< Set animasi idle/diam */
void UpdatePlayerAttack(AnimationPlayer &p);    /**< Set animasi attack */
void UpdatePlayerDeath(AnimationPlayer &p);     /**< Set animasi death/mati */
/** @} */

/**
 * @brief Update frame animasi berdasarkan delta time
 * @param p AnimationPlayer yang bakal diupdate
 * @param dt Delta time dalam detik (buat nambahin frameTime)
 */
void UpdateAnimation(AnimationPlayer &p, float dt);

/**
 * @brief Render player sprite ke layar berdasarkan state dan direction
 * @param p AnimationPlayer dengan state animasi saat ini
 */
void DrawPlayer(AnimationPlayer &p);

/**
 * @brief State animasi buat enemy entity
 */
enum EnemyState
{
    EnIDLE,   /**< Diam/standby */
    EnROAM,   /**< Jalan */
    EnCHASE,  /**< Mengejar */
    EnATTACK, /**< Nyerang */
    EnDEAD    /**< Mati */
};

/**
 *
 *
 */
struct AnimationEnemy
{
    Vector2 position; /**< Posisi entity di world (pixel) */

    EnemyState EnState; /**< State animasi saat ini (IDLE/WALK/CHASE/ATTACK/DEAD) */

    int frame;        /**< Index frame animasi yang ditampilin (wrapper dari walkFrameIndex) */
    float frameTime;  /**< Akumulator waktu buat timing animasi (dalam detik) */
    float frameSpeed; /**< Kecepatan ganti frame (durasi per frame, misal 0.1 = 10 FPS) */

    int walkFrameIndex; /**< Index frame buat mapping gambar dari texture pack */

    bool isAttacking; /**< Flag ngecek apakah entity sedang attack */
    bool isDead;      /**< Flag ngecek apakah entity udah mati */
};
