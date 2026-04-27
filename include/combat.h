#pragma once
#include "raylib.h"
#include "effects.h"
#include <vector>

class Player;

/**
 * @brief Kategorisasi gerakan serangan.
 */
enum AttackType {
    ATTACK_SLASH,   ///< Ayunan busur lebar
    ATTACK_THRUST   ///< Gerakan menusuk ke depan
};

/**
 * @brief Wadah status untuk serangan jarak dekat pemain.
 * Melacak waktu, rotasi, dan entitas yang terkena hit selama ayunan saat ini.
 */
struct SwingAttack {
    bool active = false;                ///< Apakah serangan sedang berlangsung?
    float timer = 0.0f;                 ///< Progres animasi saat ini
    float duration = 0.7f;              ///< Total durasi animasi serangan
    float startAngle = 0.0f;            ///< Rotasi awal dalam derajat
    float currentAngle = 0.0f;          ///< Rotasi dinamis selama pembaruan
    float sweepAngle = 180.0f;          ///< Total busur ayunan
    Vector2 center = {0, 0};            ///< Titik asal ayunan
    int iconX = 6;                      ///< Koordinat X ikon senjata di tileset
    int iconY = 4;                      ///< Koordinat Y ikon senjata di tileset
    std::vector<void*> damagedEntities; ///< Mencegah beberapa hit pada entitas yang sama per ayunan
    bool pressRegistered = false;       ///< Mencegah klik yang "menempel" dari status sebelumnya
    
    // Properti diferensiasi senjata
    AttackType type = ATTACK_SLASH;     ///< Tipe gerakan
    float reach = 32.0f;                ///< Jarak dari pusat ke tepi hitbox
    float breadth = 48.0f;              ///< Lebar/Area busur ayunan
    float thrustOffset = 0.0f;          ///< Pergerakan ke depan saat menusuk
    float baseAngle = 0.0f;             ///< Orientasi tetap berdasarkan arah pemain
    float damage = 25.0f;               ///< Nilai damage yang diberikan
    float knockbackForce = 1.0f;        ///< Kekuatan dorongan balik saat hit
};

/**
 * @brief Fungsi pertarungan global untuk memproses damage dan umpan balik visual.
 */
namespace Combat
{
    /**
     * @brief Pengendali tingkat tinggi untuk input dan status pertarungan pemain.
     */
    void HandleCombat(Player &player);

    /**
     * @brief Logika untuk menghidupkan kembali pemain dari status DEAD.
     */
    void HandleRevive(Player &player);
    
    /**
     * @brief Memperbarui fisika dan hitbox dari ayunan yang aktif.
     */
    void UpdateSwingAttack(Player &player, float dt);

    /**
     * @brief Me-render sprite senjata dan hitbox debug.
     */
    void DrawSwingAttack(Player &player);

    // Sistem Damage Popup (wrapper untuk Effects)
    void AddDamagePopup(Vector2 pos, float damage);
}
