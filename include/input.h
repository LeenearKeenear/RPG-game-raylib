#pragma once
#include "../lib/raylib/include/raylib.h"

// ================================================================
// Input System — Key Binding Module
// Centralized input handling buat semua action player.
//
// Fitur:
// - E         → interact
// - Arrow/WASD → movement / navigasi inventori
// - K         → uji coba player mati
// - R         → uji coba player hidup kembali (revive)
// - I         → toggle inventori
// - M         → toggle map
// - 1,2,3,4   → pilih senjata / potion slot
// - Mouse Kiri → context action (attack / minum potion / equip-unequip)
//
// Left Click context logic:
// - Jika player lagi pilih senjata (slot 1/2)   → attack
// - Jika player lagi pilih potion (slot 3/4)     → minum potion
// - Jika player lagi buka inventori              → equip/unequip
// ================================================================

// ================================================================
// Item Slot — slot aktif yang dipilih player (1-4)
// ================================================================
enum ItemSlot
{
    SLOT_NONE = 0,
    SLOT_WEAPON_1 = 1,  // key 1
    SLOT_WEAPON_2 = 2,  // key 2
    SLOT_POTION_1 = 3,  // key 3
    SLOT_POTION_2 = 4   // key 4
};

// ================================================================
// SpaceAction — tipe aksi yang dilakukan saat SPACE ditekan
// Ditentukan berdasarkan context (slot aktif / inventori terbuka)
// ================================================================
enum SpaceAction
{
    ACTION_NONE,
    ACTION_ATTACK,
    ACTION_DRINK_POTION,
    ACTION_EQUIP_UNEQUIP
};

// ================================================================
// InputState — struct yang menyimpan semua state input per frame
// Di-update sekali per frame via PollInput(), dipakai di Update()
// ================================================================
struct InputState
{
    // --- Movement ---
    bool moveUp;
    bool moveDown;
    bool moveLeft;
    bool moveRight;

    // --- Actions (pressed sekali) ---
    bool interact;       // E
    bool revive;         // R — debug: player hidup kembali
    bool toggleInventory;// I — toggle inventori
    bool toggleMap;      // M — toggle map
    bool leftClickPressed; // Left Mouse — context action

    // --- Slot Selection (pressed sekali) ---
    bool selectSlot1;    // key 1
    bool selectSlot2;    // key 2
    bool selectSlot3;    // key 3
    bool selectSlot4;    // key 4

    // --- Debug / Test (pressed sekali) ---
    bool testLoseHP;     // K
    bool testLoseMP;     // J
};

// ================================================================
// PlayerInput — class utama input system
//
// Cara pakai:
// 1. Panggil PollInput() sekali di awal frame
// 2. Cek state via getter functions
// 3. Panggil ResolveSpaceAction() untuk tahu aksi SPACE
// ================================================================
class PlayerInput
{
public:
    // poll semua key bindings — panggil sekali per frame di awal Update()
    void PollInput(void);

    // --- Getters ---
    const InputState& GetState() const { return Current; }

    // getter movement — true selama key ditekan (KeyDown)
    bool IsMoveUp()    const { return Current.moveUp; }
    bool IsMoveDown()  const { return Current.moveDown; }
    bool IsMoveLeft()  const { return Current.moveLeft; }
    bool IsMoveRight() const { return Current.moveRight; }
    bool IsMoving()    const { return Current.moveUp || Current.moveDown || Current.moveLeft || Current.moveRight; }

    // getter actions — true hanya saat key baru ditekan (KeyPressed)
    bool IsInteract()        const { return Current.interact; }
    bool IsRevive()          const { return Current.revive; }
    bool IsToggleInventory() const { return Current.toggleInventory; }
    bool IsToggleMap()       const { return Current.toggleMap; }
    bool IsLeftClickPressed() const { return Current.leftClickPressed; }

    // getter slot selection
    bool IsSelectSlot1() const { return Current.selectSlot1; }
    bool IsSelectSlot2() const { return Current.selectSlot2; }
    bool IsSelectSlot3() const { return Current.selectSlot3; }
    bool IsSelectSlot4() const { return Current.selectSlot4; }

    // getter test / debug
    bool IsTestLoseHP() const { return Current.testLoseHP; }
    bool IsTestLoseMP() const { return Current.testLoseMP; }

    // getter active slot & UI state
    ItemSlot GetActiveSlot()    const { return ActiveSlot; }
    bool IsInventoryOpen()      const { return InventoryOpen; }
    bool IsMapOpen()            const { return MapOpen; }

    // tentukan aksi left click berdasarkan context saat ini
    // return: ACTION_ATTACK / ACTION_DRINK_POTION / ACTION_EQUIP_UNEQUIP / ACTION_NONE
    SpaceAction ResolveSpaceAction() const;

    // update internal state (slot selection, toggle UI)
    // panggil setelah PollInput() di player Update()
    void UpdateState(void);

private:
    InputState Current = {};
    ItemSlot ActiveSlot = SLOT_WEAPON_1;
    bool InventoryOpen = false;
    bool MapOpen = false;
};

// global instance — diakses file lain via extern
extern PlayerInput InputInstance;
