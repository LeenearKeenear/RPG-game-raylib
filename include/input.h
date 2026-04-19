#pragma once

/**
 * @file input.h
 * @brief Input System & Key Binding Module
 *
 * Centralized input handling buat semua action player.
 */

#include "../lib/raylib/include/raylib.h"
#include "input_linkedlist.h"

// ================================================================
// Input System — Key Binding Module
// Semua key binding game dipusatin di sini.
//
// Daftar key bindings:
// - E          → interact
// - Arrow/WASD → movement / navigasi inventori
// - K          → debug: player mati
// - R          → debug: player hidup kembali (revive)
// - I          → toggle inventori
// - M          → toggle map
// - 1,2,3,4    → pilih senjata / potion slot
// - Mouse Kiri → context action (attack / minum potion / equip-unequip)
// - B          → kembali ke map sebelumnya
//
// Left Click logic berdasarkan context:
// - Kalo slot senjata (1/2) aktif   → attack
// - Kalo slot potion (3/4) aktif    → minum potion
// - Kalo inventori kebuka           → equip/unequip
// ================================================================


/*==============================================================================
 * SpaceAction Enum
 *==============================================================================*/

/**
 * @brief Tipe aksi yang dilakukan saat SPACE diteken
 * @note Ditentukan berdasarkan context (slot aktif / inventori terbuka)
 */
enum PlayerAction
{
    ACTION_NONE,         /**< Gak ada aksi */
    ACTION_ATTACK,       /**< Aksi attack (pake senjata) */
    ACTION_DRINK_POTION, /**< Aksi minum potion */
    ACTION_EQUIP_UNEQUIP /**< Aksi equip/unequip item di inventori */
};

/*==============================================================================
 * InputState Struct
 *==============================================================================*/

/**
 * @brief State input per frame
 * @note Di-update sekali per frame via PollInput(), dipakai di Update()
 */
struct InputState
{
    // --- Movement (key held / ditahan) ---
    bool moveUp;    /**< Tombol ke atas ditekan (W/Up) */
    bool moveDown;  /**< Tombol ke bawah ditekan (S/Down) */
    bool moveLeft;  /**< Tombol ke kiri ditekan (A/Left) */
    bool moveRight; /**< Tombol ke kanan ditekan (D/Right) */

    // --- Actions (key pressed sekali / baru diteken) ---
    bool interact;         /**< E - interaksi dengan object */
    bool kill;             /**< K - debug: langsung matiin player */
    bool revive;           /**< R - debug: revive player */
    bool toggleInventory;  /**< I - buka/tutup inventori */
    bool toggleMap;        /**< M - buka/tutup map */
    bool leftClickPressed; /**< Mouse Kiri - context action */
    bool goBack;           /**< Tombol buat kembali ke tempat awal */

    // --- Slot Selection (key pressed sekali) ---
    bool selectSlot1; /**< Key 1 - pilih slot senjata 1 */
    bool selectSlot2; /**< Key 2 - pilih slot senjata 2 */
    bool selectSlot3; /**< Key 3 - pilih slot potion 1 */
    bool selectSlot4; /**< Key 4 - pilih slot potion 2 */

    // --- Debug / Test (pressed sekali) ---
    bool testLoseHP;     // K

    // --- Mouse wheel ---
    float mouseWheel;   /**< Pergerakan mouse wheel frame ini */
};

// ================================================================
// PlayerInput — class utama input system
//
// Cara pakai:
// 1. Panggil PollInput() sekali di awal frame
// 2. Cek state via getter functions
// 3. Panggil ResolveAction() untuk tahu aksi yang harus dilakukan (contextual)
// ================================================================
class PlayerInput
{
public:
    PlayerInput();

    /**
     * @brief Poll semua key bindings
     * @note Panggil sekali per frame di awal Update()
     */
    void PollInput(void);

    // --- Getters ---

    /**
     * @brief Dapetin seluruh InputState
     * @return const reference ke InputState
     */
    const InputState &GetState() const { return Current; }

    // getter movement — true selama key ditekan (KeyDown)
    bool IsMoveUp() const { return Current.moveUp; }
    bool IsMoveDown() const { return Current.moveDown; }
    bool IsMoveLeft() const { return Current.moveLeft; }
    bool IsMoveRight() const { return Current.moveRight; }
    bool IsMoving() const { return Current.moveUp || Current.moveDown || Current.moveLeft || Current.moveRight; }

    // getter actions — true hanya saat key baru ditekan (KeyPressed)
    bool IsInteract()        const { return Current.interact; }
    bool IsRevive()          const { return Current.revive; }
    bool IsToggleInventory() const { return Current.toggleInventory; }
    bool IsToggleMap() const { return Current.toggleMap; }
    bool IsLeftClickPressed() const { return Current.leftClickPressed; }
    bool IsGoBack() const { return Current.goBack; }

    // getter slot selection
    bool IsSelectSlot1() const { return Current.selectSlot1; }
    bool IsSelectSlot2() const { return Current.selectSlot2; }
    bool IsSelectSlot3() const { return Current.selectSlot3; }
    bool IsSelectSlot4() const { return Current.selectSlot4; }

    // getter test / debug
    bool IsTestLoseHP() const { return Current.testLoseHP; }

    // getter active slot & UI state
    ItemSlot GetActiveSlot() const { return ActiveSlot; }
    bool IsInventoryOpen() const { return InventoryOpen; }
    bool IsMapOpen() const { return MapOpen; }

    /**
     * @brief Tentukan aksi left click berdasarkan context saat ini
     * @return ACTION_ATTACK / ACTION_DRINK_POTION / ACTION_EQUIP_UNEQUIP / ACTION_NONE
     */
    PlayerAction ResolveAction() const;

    /**
     * @brief Update internal state (slot selection, toggle UI)
     * @note Panggil setelah PollInput() di player Update()
     */
    void UpdateState(void);

private:
    InputState Current = {};         /**< State input current frame */
    ItemSlot ActiveSlot = SLOT_WEAPON_1; /**< Slot yang lagi aktif (1-4) */
    bool InventoryOpen = false;      /**< Flag apakah inventori kebuka */
    bool MapOpen = false;            /**< Flag apakah map kebuka */

    HotbarList doubleLinkedList;     /**< Linked list untuk navigasi hotbar */
};

/*==============================================================================
 * Global Input Instance
 *==============================================================================*/

/** Global instance input system - diakses file lain via extern */
extern PlayerInput InputInstance;