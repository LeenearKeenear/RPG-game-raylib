#pragma once

#include "../lib/raylib/include/raylib.h"
#include "map.h"
#include "screen.h"
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
    /** @brief Handle input gerak player */
    void HandleMovement(Player &player);
    /** @brief Update kamera mengikuti player */
    void UpdateCamera(Player &player);
    /** @brief Cek apakah player bisa bergerak ke posisi */
    bool CanMove(Player &player, Vector2 newPos);
}

/**
 * @brief Namespace untuk logika pertarungan dan manajemen kesehatan pemain.
 */
namespace Combat
{
    void Update(Player &player);
    /** @brief Handle revive player */
    void HandleRevive(Player &player);
    void UpdateSwingAttack(Player &player, float dt);
    void DrawSwingAttack(Player &player);
}

/**
 * @brief Namespace untuk interaksi lingkungan dan raycasting.
 */
namespace Interaction
{
    /** @brief Handle interaksi player */
    void HandleInteractions(Player &player);
    /** @brief Update raycast interaksi */
    void UpdateRaycast(Player &player);
    /** @brief Cek interaksi dengan pintu */
    void CheckDoors(Player &player);
    /** @brief Cek interaksi dengan props */
    void CheckProps(Player &player);
}

/**
 * @brief Namespace untuk hotbar dan penggunaan item.
 */
namespace Inventory
{
    /** @brief Handle aksi inventory player */
    void HandleInventoryActions(Player &player);
    /** @brief Gunakan potion dari slot */
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
    Animation Anim;                // Pengontrol animasi
    bool pendingSwitchMap = false; // Flag untuk memicu transisi map
    std::string pendingMapPath;    // Path map tujuan
    std::string pendingDoorName;   // Nama pintu tujuan di map baru
    bool pendingGoBack = false;    // Flag untuk kembali ke map sebelumnya
    GameState *State = nullptr;    // Pointer ke status game global

    RayCast Ray;                                         // Raycast untuk interaksi
    RayHitResult LastHit;                                // Data dari tabrakan raycast terakhir
    std::vector<Rectangle> CollisionRects;               // Tile tabrakan yang aktif
    std::vector<std::vector<Vector2>> CollisionPolygons; // Bentuk poligon tabrakan yang aktif
    Vector2 Velocity = {0, 0};                           // Vektor kecepatan gerak saat ini

    /**
     * @brief Inisialisasi status pemain dan lokasi spawn.
     */
    void Init(GameState *state, const char *spawnObjectName = SPAWN_OBJECT_NAME);
    /** @brief Update player tiap frame */
    void Update() override;
    /** @brief Render player */
    void Render(void) override;
    /** @brief Apply damage ke player */
    void TakeDamage(float amount, Vector2 knockback = {0, 0}) override;
    /** @brief Set posisi player */
    void SetPosition(Vector2 pos) { Position = pos; }

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
    /** @brief Set mana player */
    void SetMana(float m) { Mana = m; }
    /** @brief Set item di hotbar */
    void SetHotbarItem(int index, InventoryItem item) { Hotbar[index] = item; }

    /** @brief Draw aim indicator */
    void DrawAimIndicator();

    /** @brief Cek apakah player hidup */
    bool IsAlive() const override { return !Anim.isDead; }
    /** @brief Get nama player */
    const char *GetName() { return Name; }
    /** @brief Get health player */
    float GetHealth() { return Health; }
    /** @brief Get max health player */
    float GetMaxHealth() { return MaxHealth; }
    /** @brief Get mana player */
    float GetMana() { return Mana; }
    /** @brief Get max mana player */
    float GetMaxMana() { return MaxMana; }

    /** @brief Get sudut raycast */
    float GetRayCastAngle() const { return RayCastAngle; }
    /** @brief Get sudut drop item */
    float GetItemDropAngle() const { return RayCastAngleItemDrop; }
    /** @brief Get sudut drop belakang */
    float GetItemDropAngleBack() const { return RayCastAngleItemDropBack; }
    /** @brief Get range interaksi */
    float GetInteractRange() const { return INTERACT_RANGE; }

    /** @brief Get posisi player */
    Vector2 GetPosition() { return Position; }

    /** @brief Get lebar hitbox */
    float GetHitboxWidth() { return HitboxWidth; }
    /** @brief Get tinggi hitbox */
    float GetHitboxHeight() { return HitboxHeight; }
    /** @brief Get offset X hitbox */
    float GetHitboxOffsetX() { return HitboxOffsetX; }
    /** @brief Get offset Y hitbox */
    float GetHitboxOffsetY() { return HitboxOffsetY; }
    Vector2 GetCenter() const override
    {
        return {
            Position.x + HitboxOffsetX + HitboxWidth / 2,
            Position.y + HitboxOffsetY + HitboxHeight / 2};
    }
    Rectangle GetHitbox() const override
    {
        return {
            Position.x + HitboxOffsetX,
            Position.y + HitboxOffsetY,
            HitboxWidth,
            HitboxHeight};
    }

    /** @brief Get radius magnet pickup */
    float GetMagnetRadius() { return MagnetRadius; }
    /** @brief Get speed item pickup */
    float GetItemSpeed() { return ItemSpeed; }

    /** @brief Get item hotbar */
    InventoryItem &GetHotbarItem(int index) { return Hotbar[index]; }
    /** @brief Get item hotbar (const) */
    const InventoryItem &GetHotbarItem(int index) const { return Hotbar[index]; }
    /** @brief Get item bag */
    InventoryItem &GetBagItem(int index) { return Bag[index]; }
    /** @brief Get item bag (const) */
    const InventoryItem &GetBagItem(int index) const { return Bag[index]; }
    /** @brief Get kapasitas maksimum bag */
    int GetMaxBag() const { return MaxBag; }
    /** @brief Get kapasitas maksimum hotbar */
    int GetMaxHotbar() const { return MaxHotbar; }
    /** @brief Get kapasitas maksimum inventory */
    int GetMaxInventory() const { return MaxInventory; }

    // combat stat
    Combat::Attack attack = {};  ///< Data status serangan saat ini
    float Mana = 100.0f;         ///< Poin mana saat ini
    float MaxMana = 100.0f;      ///< Poin mana maksimum
    float ManaRegenTimer = 0.0f; ///< Timer untuk jeda pemulihan mana
    const float ManaRegenDelay = 2.0f;
    const float ManaRegenRate = 10.0f;
    const float AttackManaCost = 10.0f;

    // movement
    float Speed = 20.0f;          // Kecepatan gerak dasar 3.7
    float DashSpeed = 0.0f;       // current dash speed tambahan
    float DashMaxSpeed = 4.0f;    // max dash speed
    float DashDecel = 0.06f;      // lerp factor deselerasi
    float DashCooldown = 0.0f;    // timer cooldown
    float DashCooldownMax = 0.6f; // durasi cooldown
    float DashDuration = 0.0f;
    float DashDurationMax = 0.1f;
    float DashManaCost = 7.0f;
    bool IsDashing = false;
    bool IsMoving = false;
    bool canInteract = false;

    // Feedback visual/fisika
    float HitFlashTimer = 0.0f;         ///< Durasi efek kilatan saat terkena hit
    Vector2 KnockbackVelocity = {0, 0}; ///< Gaya dorong balik (knockback) yang sedang diterapkan
    Direction LastHorizDir = RIGHT;     ///< Arah horizontal terakhir pemain (LEFT atau RIGHT)

private:
    const char *Name = "Player Name";
    bool isInitialized = false;

    Rectangle GetPlayerHitboxAtPosition(Vector2 position);
    bool CanMove(Vector2 NewPos);

    const float INTERACT_RANGE = 32.0f;

    // hitbox
    float HitboxWidth = 16.0f;
    float HitboxHeight = 12.0f;
    float HitboxOffsetX = 8.0f;
    float HitboxOffsetY = 14.0f;

    // inventory sistem
    InventoryItem Hotbar[4];                     // Item akses cepat (hotbar) pemain
    InventoryItem Bag[12];                       // Tas penyimpanan utama pemain (inventory, 5x5 grid)
    const int MaxBag = 12;                       // jumlah maksimum bag. nilainya harus sama dengan bag
    const int MaxHotbar = 4;                     // jumlah maksimum hotbar. nilainya harus sama dengan hotbar
    const int MaxInventory = MaxBag + MaxHotbar; // keseluruhan inventory

    // magnet/pickup fields
    float MagnetRadius = 70.0f;        // default 70
    float ItemSpeed = 300.0f;          // default 300
    const float RayCastAngle = 0.000f; // cos(90°) — area pandang ±90° dari arah hadap (180 derajat)

    // item drop angle
    const float RayCastAngleItemDrop = 0.000f;    // ±90° dari arah hadap
    const float RayCastAngleItemDropBack = 37.0f; // zona terlarang belakang (derajat)

    /** @brief Handle action input */
    void HandleAction(void);
};

/** @brief Instance global player */
extern Player PlayerInstance;
