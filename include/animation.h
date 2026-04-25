#pragma once

#include "raylib.h"
#include "tiles.h"

/**
 * @brief Status animasi yang mungkin untuk suatu entitas.
 */
enum State
{
    IDLE,
    WALK,
    ATTACK,
    DEAD
};

struct AnimationSet;

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