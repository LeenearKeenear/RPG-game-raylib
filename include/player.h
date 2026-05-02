#pragma once

#include "../lib/raylib/include/raylib.h"
#include "map.h"
#include "screen.h"
#include "tiles.h"
#include "animation.h"
#include "inventory.h"
#include "entity.h"
#include "input.h"
#include "mapLogic.h"

class Player;

/**
 * @brief Namespace untuk logika pergerakan pemain dan kamera.
 */
namespace Movement
{
    void HandleMovement(Player &player);
    void UpdateCamera(Player &player);
    bool CanMove(Player &player, Vector2 newPos);
}

/**
 * @brief Namespace untuk logika pertarungan dan manajemen kesehatan pemain.
 */
namespace Combat
{
    void HandleCombat(Player &player);
    void HandleRevive(Player &player);
}

/**
 * @brief Namespace untuk interaksi lingkungan dan raycasting.
 */
namespace Interaction
{
    void HandleInteractions(Player &player);
    void UpdateRaycast(Player &player);
    void CheckDoors(Player &player);
    void CheckProps(Player &player);
}

/**
 * @brief Namespace untuk hotbar dan penggunaan item.
 */
namespace Inventory
{
    void HandleInventoryActions(Player &player);
    void UsePotion(Player &player, int slotIndex);
}

#include "combat.h"

/**
 * @brief Kelas utama karakter pemain.
 * Menangani integrasi input, pergerakan, status pertarungan, dan data interaksi.
 */
class Player : public Entity
{
public:
    /**
     * @brief Inisialisasi status pemain dan lokasi spawn.
     */
    void Init(GameState *state, const char *spawnObjectName = SPAWN_OBJECT_NAME);

    void Update() override;
    void Render(void) override;
    void TakeDamage(float amount, Vector2 knockback = {0, 0}) override;

    SwingAttack Swing = {0}; ///< Data status serangan saat ini

    Vector2 Velocity = {0, 0};   ///< Vektor kecepatan gerak saat ini
    float Speed = 4.0f;          ///< Kecepatan gerak dasar
    float Mana = 100.0f;         ///< Poin mana saat ini
    float MaxMana = 100.0f;      ///< Poin mana maksimum
    float ManaRegenTimer = 0.0f; ///< Timer untuk jeda pemulihan mana
    const float ManaRegenDelay = 2.0f;
    const float ManaRegenRate = 10.0f;
    const float AttackManaCost = 10.0f;

    Animation Anim;                ///< Pengontrol animasi
    bool pendingSwitchMap = false; ///< Flag untuk memicu transisi map
    std::string pendingMapPath;    ///< Path map tujuan
    std::string pendingDoorName;   ///< Nama pintu tujuan di map baru
    bool pendingGoBack = false;    ///< Flag untuk kembali ke map sebelumnya
    InventoryItem Hotbar[4];       ///< Item akses cepat (hotbar) pemain
    InventoryItem Bag[20];         ///< Tas penyimpanan utama pemain (inventory, 5x5 grid)
    const int MaxBag = 20; ///< jumlah maksimum bag. nilainya harus sama dengan bag
    const int MaxHotbar = 4;    ///< jumlah maksimum hotbar. nilainya harus sama dengan hotbar
    const int MaxInventory = MaxBag + MaxHotbar; ///< keseluruhan inventory

    float HitboxWidth = 16.0f;
    float HitboxHeight = 12.0f;
    float HitboxOffsetX = 8.0f;
    float HitboxOffsetY = 14.0f;

    Rectangle GetHitbox() const override
    {
        return {
            Position.x + HitboxOffsetX,
            Position.y + HitboxOffsetY,
            HitboxWidth,
            HitboxHeight};
    }

    /** @return Posisi player dalam pixel */
    Vector2 GetPosition() { return Position; }
    void SetPosition(Vector2 pos) { Position = pos; }

    std::vector<Rectangle> CollisionRects;               ///< Tile tabrakan yang aktif
    std::vector<std::vector<Vector2>> CollisionPolygons; ///< Bentuk poligon tabrakan yang aktif

    RayCast Ray;          ///< Raycast untuk interaksi
    RayHitResult LastHit; ///< Data dari tabrakan raycast terakhir
    const float INTERACT_RANGE = 32.0f;

    // Getter untuk modul logika eksternal
    bool IsAlive() const override { return !Anim.isDead; }
    float GetHealth() { return Health; }
    float GetMaxHealth() { return MaxHealth; }
    float GetMana() { return Mana; }
    float GetMaxMana() { return MaxMana; }
    const char *GetName() { return Name; }
    RayHitResult GetLastHit() { return LastHit; }
    float GetHitboxWidth() { return HitboxWidth; }
    float GetHitboxHeight() { return HitboxHeight; }
    float GetHitboxOffsetX() { return HitboxOffsetX; }
    float GetHitboxOffsetY() { return HitboxOffsetY; }
    float GetINTERACT_RANGE() { return INTERACT_RANGE; }
    float GetSpeed() { return Speed; }

    // pickup getters
    float GetMagnetRadius() { return MagnetRadius; }
    float GetItemSpeed() { return ItemSpeed; }

    // hitbox getter — override dari Entity::GetCenter()
    Vector2 GetCenter() const override
    {
        return {
            Position.x + HitboxOffsetX + HitboxWidth / 2,
            Position.y + HitboxOffsetY + HitboxHeight / 2};
    }

    // Hotbar management
    InventoryItem GetHotbarItem(int index) { return Hotbar[index]; }

    /**
     * @brief Memperbarui kesehatan dengan pemeriksaan batas.
     */
    void SetHealth(float h)
    {
        Health = h;
        if (Health < 0)
            Health = 0;
        if (Health > MaxHealth)
            Health = MaxHealth;
    }
    void SetMana(float m) { Mana = m; }
    void SetHotbarItem(int index, InventoryItem item) { Hotbar[index] = item; }

    GameState *State = nullptr; ///< Pointer ke status game global

    // Feedback visual/fisika
    float HitFlashTimer = 0.0f;         ///< Durasi efek kilatan saat terkena hit
    Vector2 KnockbackVelocity = {0, 0}; ///< Gaya dorong balik (knockback) yang sedang diterapkan

    // Logic methods — definisi: src/player.cpp
    void DrawAimIndicator();
    float GetRayCastAngle() const { return RayCastAngle; }

private:
    const char *Name = "Player Name";
    bool isInitialized = false;

    Rectangle GetPlayerHitboxAtPosition(Vector2 position);
    bool CanMove(Vector2 NewPos);

    // magnet/pickup fields
    float MagnetRadius = 70.0f;        // default 70
    float ItemSpeed = 300.0f;          // default 300
    const float RayCastAngle = 0.600f; ///< cos(45°) — area pandang ±45° dari arah hadap

    // Action handler — definisi: src/player.cpp
    void HandleAction(void);
};

extern Player PlayerInstance;
