#pragma once

/**
 * @file player.h
 * @brief Player System Module
 *
 * Handle semua behavior player: movement, collision, render, camera.
 * Ini inti dari gameplay player character.
 */

#include "../lib/raylib/include/raylib.h"
#include "map.h"
#include "screen.h"
#include "animation.h"
#include "input.h"
#include "inventory.h"


// ================================================================
// Player Class
// Handle semua behavior player: movement, collision, render
//
// Workflow collision:
// - Collision dibaca dari object layer "collision" di Tiled
// - Setiap Rectangle / Polygon di layer itu dianggap solid/blocked
// - CanMove() ngecek apakah posisi baru player nabrak salah satu shape itu
//
// Workflow world bound:
// - World boundary custom dibaca dari object layer "map_bound" di Tiled
// - Kalau layer boundary tidak ada / tidak punya polygon,
//   CanMove() fallback ke rectangle ukuran map
//
// Workflow spawn:
// - Spawn point dibaca dari object bernama "spawn" di Tiled
// - Init() otomatis taruh player di posisi spawn
// ================================================================

/*==============================================================================
 * Player Class
 *==============================================================================*/

/**
 * @brief Class utama buat handle player character
 *
 * Nyediain semua fungsi yang berhubungan sama player:
 * - Movement & collision detection
 * - Animasi (jalan, idle, attack, mati)
 * - Camera follow
 * - Interaksi dengan object (door, dll)
 * - Handle input dari PlayerInput
 */
class Player
{
public:
    // ================================================================
    // Initialization
    // ================================================================

    /**
     * @brief Inisialisasi player
     * @param spawnObjectName Nama object spawn point di Tiled (default: SPAWN_OBJECT_NAME)
     * @note Load texture, baca spawn & collision dari Tiled, setup animasi awal
     */
    void Init(const char *spawnObjectName = SPAWN_OBJECT_NAME);

    // ================================================================
    // Update & Render
    // ================================================================

    /**
     * @brief Handle input dan movement per frame
     * @note Cek collision sebelum apply posisi baru
     *       Juga handle attack, death, revive, dll
     */
    void Update(void);

    /**
     * @brief Render sprite player di posisi world saat ini
     * @note Panggil DrawPlayer() dari animation system
     */
    void Render(void);

    /**
     * @brief Handle camera follow player dengan clamp ke world bounds
     * @note Camera bakal ngikutin player tapi gak bakal keluar dari batas map
     */
    void PlayerCamera(void);

    /**
     * @brief Wrapper per frame — dipanggil dari UpdateLogicAll()
     * @note Urutan: Update() → PlayerCamera()
     */
    void Tick(void);

    // ================================================================
    // Getters
    // ================================================================

    /** @return Posisi player dalam pixel */
    Vector2 GetPosition() { return Position; }

    /** @return Speed player — dipake debug panel */
    float GetSpeed() { return Speed; }

    /** @return true kalo player masih hidup, false kalo udah mati */
    bool IsAlive() { return !Anim.isDead; }

    // hitbox getters — dipake collision dan debug panel
    float GetHitboxWidth() { return HitboxWidth; }
    float GetHitboxHeight() { return HitboxHeight; }
    float GetHitboxOffsetX() { return HitboxOffsetX; }
    float GetHitboxOffsetY() { return HitboxOffsetY; }

    // ================================================================
    // Frustum Culling
    // ================================================================

    /**
     * @brief Hitung range tile yang visible di layar berdasarkan camera viewport
     * @return TileRange berisi minX, minY, maxX, maxY
     * @note Ini adalah inti logic frustum culling — dipake oleh RenderMap()
     */
    TileRange GetVisibleTileRange(void);

    // health getters
    float GetHealth() { return Health; }
    float GetMaxHealth() { return MaxHealth; }
    void SetHealth(float h) { Health = h; }

    // mana getters
    float GetMana() { return Mana; }
    float GetMaxMana() { return MaxMana; }
    void SetMana(float m) { Mana = m; }

    // info getters
    const char* GetName() { return Name; }

    // ================================================================
    // Public Members
    // ================================================================

    AnimationPlayer Anim; /**< Data animasi player (state, frame, dll) */

    bool pendingSwitchMap = false; /**< Flag nunggu ganti map */
    std::string pendingMapPath;    /**< Path map tujuan */
    std::string pendingDoorName;   /**< Nama pintu tujuan di map baru */
    bool pendingGoBack = false;    /**< Flag nunggu aksi go back ke map sebelumnya */

    // Hotbar management
    InventoryItem GetHotbarItem(int index) { return Hotbar[index]; }
    void SetHotbarItem(int index, InventoryItem item) { Hotbar[index] = item; }
    void UsePotion(int slotIndex);

private:
    // ================================================================
    // Private Methods
    // ================================================================

    /**
     * @brief Dapetin hitbox player di posisi tertentu
     * @param position Posisi yang mau dicek (biasanya posisi baru sebelum movement)
     * @return Rectangle hitbox dengan offset yang udah di-apply
     */
    Rectangle GetPlayerHitboxAtPosition(Vector2 position);

    /**
     * @brief Cek apakah posisi baru player valid (gak nabrak collision & masih di dalam map)
     * @param NewPos Posisi baru yang mau dicek
     * @return false kalo nabrak collision / keluar bound, true kalo aman
     */
    bool CanMove(Vector2 NewPos);

    /**
     * @brief Cek interaksi dengan door/pintu dan trigger switch map kalo perlu
     */
    void CheckDoorInteraction(void);

    /**
     * @brief Handle aksi left click berdasarkan context (slot aktif / inventori)
     * @note Panggil ResolveSpaceAction() dari PlayerInput
     */
    void HandleSpaceAction(void);

    /**
     * @brief Revive player — reset state dari DEAD ke IDLE
     * @note Dipanggil pas tekan tombol R (debug)
     */
    void HandleRevive(void);

    // ================================================================
    // Private Members
    // ================================================================

    Vector2 Position;      /**< Posisi player di world (pixel) */
    Vector2 Velocity;      /**< Kecepatan player (belum dipake maksimal) */
    int TileSize = 32;     /**< Ukuran tile dalam pixel */
    float Speed = 4.0f;    /**< Kecepatan gerak player (pixel per frame) */
    Texture2D CharTexture; /**< Texture sprite player */
    const char* Name = "Player Name";

    // ukuran hitbox player bisa diperkecil dari sprite biar movement
    // terasa lebih enak dan gak gampang nyangkut di sudut/object.
    float HitboxWidth = 16.0f;   /**< Lebar hitbox player */
    float HitboxHeight = 12.0f;  /**< Tinggi hitbox player */
    float HitboxOffsetX = 8.0f;  /**< Offset X hitbox dari posisi (makin besar makin ke kanan) */
    float HitboxOffsetY = 14.0f; /**< Offset Y hitbox dari posisi (makin besar makin ke bawah) */

    // collision data dari Tiled — diisi pas Init()
    std::vector<Rectangle> CollisionRects;               /**< Rectangle collision dari object layer */
    std::vector<std::vector<Vector2>> CollisionPolygons; /**< Polygon collision dari object layer */

    // custom world boundary polygon dari Tiled — diisi pas Init()
    // kalo kosong, CanMove() fallback ke rectangle ukuran map
    std::vector<Vector2> WorldBoundaryPolygon; /**< Boundary polygon custom */

    // health player
    float Health = 100.0f;
    float MaxHealth = 100.0f;

    // mana/energy player
    float Mana = 100.0f;
    float MaxMana = 100.0f;
    float ManaRegenTimer = 0.0f;
    const float ManaRegenDelay = 2.0f;
    const float ManaRegenRate = 10.0f; // per second
    const float AttackManaCost = 10.0f;

    // handle action berdasarkan context (slot aktif / inventori)
    void HandleAction(void);

    // Hotbar slots (1-4)
    InventoryItem Hotbar[4];
};


/*==============================================================================
 * Global Player Instance
 *==============================================================================*/

/** Global instance player — bisa diakses file lain via extern */
extern Player PlayerInstance;
