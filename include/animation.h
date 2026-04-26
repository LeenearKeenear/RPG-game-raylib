#pragma once

#include "raylib.h"
#include "tiles.h"

/*==============================================================================
 * Animation Logic (Data-Driven)
 *==============================================================================*/

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
 * Animation Functions
 *==============================================================================*/

/**
 * @brief Render satu tile dari spritesheet ke posisi world dengan ukuran kecil
 */
void DrawSmallSprite(TextureAsset slot, Vector2 sheetCoord, Vector2 worldPos, float scale);

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
     * @param timer Waktu yang sudah berjalan.
     * @param duration Durasi total animasi.
     * @return Nilai alpha antara 0.0f dan 1.0f.
     */
    float CalculateFadeOut(float timer, float duration);

    /**
     * @brief Menghitung posisi offset vertikal yang terus naik.
     * @param currentOffset Offset saat ini.
     * @param speed Kecepatan naik (pixel per detik).
     * @param dt Delta time.
     * @return Offset baru.
     */
    float CalculateFloatOffset(float currentOffset, float speed, float dt);

    /**
     * @brief Menentukan apakah objek harus digambar (efek kedip).
     * @param timer Waktu yang sudah berjalan.
     * @param frequency Frekuensi kedipan.
     * @return True jika harus digambar, False jika tidak.
     */
    bool ShouldBlink(float timer, float frequency);

    /**
     * @brief Menerapkan logika fisika sederhana (gravitasi & friksi) pada posisi.
     * @param pos Referensi ke posisi.
     * @param vel Referensi ke velocity.
     * @param gravity Nilai gravitasi.
     * @param friction Nilai friksi (0.0 - 1.0).
     * @param dt Delta time.
     */
    void ApplyPhysics(Vector2& pos, Vector2& vel, float gravity, float friction, float dt);

    /**
     * @brief Menggerakkan posisi ke arah target dengan kecepatan tertentu.
     * @param current Posisi saat ini.
     * @param target Posisi tujuan.
     * @param speed Kecepatan gerak.
     * @param dt Delta time.
     * @return Posisi baru.
     */
    Vector2 LerpTowards(Vector2 current, Vector2 target, float speed, float dt);

    /**
     * @brief Menghitung offset maju-mundur untuk animasi tusukan (Thrust).
     * @param progress Progress animasi (0.0 - 1.0).
     * @param maxOffset Jarak maksimum tusukan.
     * @return Offset saat ini.
     */
    float CalculateThrustOffset(float progress, float maxOffset);

    /**
     * @brief Menghitung sudut rotasi untuk animasi ayunan (Slash).
     * @param progress Progress animasi (0.0 - 1.0).
     * @param startAngle Sudut awal ayunan.
     * @param sweepAngle Total sudut yang ditempuh.
     * @return Sudut rotasi saat ini.
     */
    float CalculateSlashRotation(float progress, float startAngle, float sweepAngle);
}

