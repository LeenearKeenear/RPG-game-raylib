#pragma once

#include "../lib/raylib/include/raylib.h"

/*==============================================================================
 * Tile & Texture Constants / Structures (Moved from tiles.h)
 *==============================================================================*/

constexpr int TILE_SIZE = 32;   ///< Ukuran standar tile dalam pixel
constexpr int TILE_GAP = 4;     ///< Jarak antar tile dalam tileset (jika ada)
constexpr int MAX_TEXTURES = 6; ///< Jumlah maksimum tekstur yang dimuat secara bersamaan

extern Texture2D TexturesMap[MAX_TEXTURES];

/**
 * @brief Pengidentifikasi (ID) untuk aset tekstur yang dimuat ke memori.
 * @note TEXTURE_TILEMAP digunakan oleh map renderer (map.cpp)
 */
enum TextureAsset
{
    TEXTURE_TILEMAP = 0, ///< Tileset untuk rendering map
    TEXTURE_KNIGHT,      ///< Sprite pemain/karakter
    TEXTURE_ITEMS,       ///< Ikon dan item koleksi
    TEXTURE_ENEMIES      ///< Sprite musuh
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
 * @brief Memuat tekstur ke dalam slot tertentu.
 */
void LoadTileTexture(TextureAsset Slot, const char *Path);

/**
 * @brief Helper untuk mendapatkan rectangle sumber dari koordinat grid.
 */
Rectangle GetFrame(int frameX, int frameY);

/**
 * @brief Helper untuk merender tile langsung ke layar tanpa mengakses TexturesMap secara manual.
 */
void DrawTileTexture(TextureAsset slot, int frameX, int frameY, Rectangle dest, Vector2 origin = {0, 0}, float rotation = 0.0f, Color tint = WHITE);

/*==============================================================================
 * Animation State & Direction
 *==============================================================================*/

/** State animasi buat entity (player, enemy, dll) */
enum State
{
    IDLE,
    WALK,
    ATTACK,
    DEAD
};

/**
 * @brief Arah mata angin untuk orientasi animasi.
 */
enum Direction
{
    LEFT,
    RIGHT,
    DOWN,
    UP
};

struct AnimationSet;

/**
 * @brief Konfigurasi untuk urutan animasi tertentu.
 */
struct AnimationConfig
{
    int row;             ///< Baris pada tileset
    int startFrame;      ///< Kolom frame awal
    int frameCount;      ///< Jumlah frame dalam urutan
    float speed;         ///< Durasi setiap frame dalam detik
    bool loop;           ///< Apakah animasi harus diulang?
    int pattern[8];      ///< Pola urutan frame kustom (opsional)
    int patternCount;    ///< Jumlah frame dalam pola kustom
};

/**
 * @brief Status runtime dari sebuah instansi animasi.
 */
struct Animation
{
    Vector2 position;            ///< Posisi tempat animasi di-render
    State state;                 ///< Status logika saat ini
    Direction direction;         ///< Arah hadap saat ini
    int currentFrame;            ///< Indeks frame saat ini dalam urutan
    float timer;                 ///< Waktu yang telah berlalu pada frame saat ini
    int walkFrameIndex;          ///< Indeks khusus untuk pola berjalan
    bool isAttacking;            ///< Flag untuk memblokir animasi lain saat menyerang
    bool isDead;                 ///< Flag untuk mengunci animasi pada status DEAD
    const AnimationConfig *currentConfig; ///< Pointer ke konfigurasi yang aktif
    const AnimationSet *animSet;  ///< Set yang berisi semua animasi yang tersedia
};

/**
 * @brief Wadah untuk semua animasi milik tipe entitas tertentu.
 */
struct AnimationSet
{
    AnimationConfig configs[4][4]; // [Status][Arah]
};

/*==============================================================================
 * Tile Rendering (Legacy helpers)
 *==============================================================================*/

/**
 * @brief Jenis tile yang ada di spritesheet (untuk RenderTilePNG)
 * @note Digunakan oleh RenderTilePNG() saja
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
 */
typedef struct
{
    TileCoordinate CoordID; /**< Posisi tile di spritesheet */
    bool IsWalkable;        /**< True kalo player/enemy bisa lewat */
    bool HasInteraction;    /**< True kalo tile punya event interaksi */
} TileDefinition;

/**
 * @brief Render satu tile dari spritesheet ke posisi world (legacy helper)
 */
void RenderTilePNG(int pos_x, int pos_y, TileType Type, float Rotation, TextureAsset Slot);

/**
 * @brief Render satu tile dari spritesheet ke posisi world dengan ukuran kecil
 */
void DrawSmallSprite(TextureAsset slot, Vector2 sheetCoord, Vector2 worldPos, float scale);

/*==============================================================================
 * Animation Functions
 *==============================================================================*/

/**
 * @brief Melanjutkan progres status animasi berdasarkan langkah waktu (dt).
 */
void UpdateAnimation(Animation &anim, float dt);

/**
 * @brief Me-render animasi pada frame saat ini.
 */
void DrawAnimation(const Animation &anim, TextureAsset texture, Color tint = WHITE);

/**
 * @brief Berpindah ke status animasi dan arah yang baru.
 */
void PlayAnimation(Animation &anim, State newState, Direction newDir, const AnimationSet &set);

// Set animasi yang telah ditentukan untuk berbagai tipe entitas
extern const AnimationSet PlayerAnimationSet;
extern const AnimationSet SlimeAnimationSet;
extern const AnimationSet SkeletonAnimationSet;
extern const AnimationSet WolfAnimationSet;

/*==============================================================================
 * Generic Animation Effects (UI & Feedback)
 *==============================================================================*/

namespace AnimEffects
{
    /**
     * @brief Menghitung nilai alpha untuk efek fade-out linear.
     */
    float CalculateFadeOut(float timer, float duration);

    /**
     * @brief Menghitung posisi offset vertikal yang terus naik.
     */
    float CalculateFloatOffset(float currentOffset, float speed, float dt);

    /**
     * @brief Menentukan apakah objek harus digambar (efek kedip).
     */
    bool ShouldBlink(float timer, float frequency);

    /**
     * @brief Menerapkan logika fisika sederhana (gravitasi & friksi) pada posisi.
     */
    void ApplyPhysics(Vector2& pos, Vector2& vel, float gravity, float friction, float dt);

    /**
     * @brief Menggerakkan posisi ke arah target dengan kecepatan tertentu.
     */
    Vector2 LerpTowards(Vector2 current, Vector2 target, float speed, float dt);

    /**
     * @brief Menghitung offset maju-mundur untuk animasi tusukan (Thrust).
     */
    float CalculateThrustOffset(float progress, float maxOffset);

    /**
     * @brief Menghitung sudut rotasi untuk animasi ayunan (Slash).
     */
    float CalculateSlashRotation(float progress, float startAngle, float sweepAngle);
}

// Fungsi lama — masih dipakai oleh HandleAction() di player.cpp
// Definisi: src/animation.cpp
void UpdatePlayerAttack(Animation &p);
