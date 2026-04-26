#pragma once
#include "../lib/raylib/include/raylib.h"

/**
 * @brief Kelas dasar untuk semua objek interaktif di dalam game.
 * Menyediakan properti dasar seperti posisi, kesehatan (health), dan manajemen lifecycle.
 */
class Entity
{
public:
    Vector2 Position = {0, 0};  ///< Posisi dalam koordinat dunia (world space)
    float Health = 100.0f;      ///< Poin kesehatan saat ini
    float MaxHealth = 100.0f;   ///< Batas poin kesehatan maksimum
    bool IsActive = true;       ///< Flag untuk menentukan apakah entitas harus di-update/render

    virtual ~Entity() {}

    /**
     * @brief Logika update untuk entitas. Dipanggil satu kali setiap frame.
     */
    virtual void Update() = 0;

    /**
     * @brief Render entitas ke layar. Dipanggil satu kali setiap frame.
     */
    virtual void Render() = 0;

    /**
     * @brief Mendapatkan posisi entitas saat ini.
     * @return Vector2 posisi
     */
    Vector2 GetPosition() const { return Position; }

    /**
     * @brief Memeriksa apakah entitas masih hidup.
     * @return true jika Health > 0
     */
    virtual bool IsAlive() const { return Health > 0; }

    /**
     * @brief Mendapatkan hitbox tabrakan entitas.
     * @return Rectangle yang merepresentasikan hitbox
     */
    virtual Rectangle GetHitbox() const { return { Position.x, Position.y, 32, 32 }; }

    /**
     * @brief Mendapatkan titik tengah (center) dari entitas berdasarkan hitbox-nya.
     * @return Vector2 titik tengah
     */
    virtual Vector2 GetCenter() const {
        Rectangle hb = GetHitbox();
        return { hb.x + hb.width / 2.0f, hb.y + hb.height / 2.0f };
    }

    /**
     * @brief Memberikan damage ke entitas.
     * @param amount Nilai damage yang akan dikurangi
     * @param knockback Vektor opsional untuk gaya dorong balik (knockback)
     */
    virtual void TakeDamage(float amount, Vector2 knockback = {0, 0}) {
        (void)knockback;
        Health -= amount;
        if (Health < 0) Health = 0;
        if (Health > MaxHealth) Health = MaxHealth;
    }
};
