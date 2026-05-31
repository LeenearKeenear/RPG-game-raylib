#pragma once

#include "../lib/raylib/include/raylib.h"
#include "inputLinkedList.h"

/**
 * @brief Aksi tingkat tinggi yang dipetakan dari input mentah.
 */
enum PlayerAction
{
    ACTION_NONE,
    ACTION_ATTACK,
    ACTION_DRINK_POTION,
    ACTION_EQUIP_UNEQUIP,
    ACTION_DROP_ITEM,
};

/**
 * @brief Cuplikan (snapshot) status input mentah untuk satu frame.
 */
struct InputState
{
    bool moveUp;
    bool moveDown;
    bool moveLeft;
    bool moveRight;

    bool interact;
    bool kill;
    bool revive;
    bool toggleInventory;
    bool toggleMap;
    bool leftClickPressed;
    bool rightClickPressed;
    bool leftClickReleased;
    bool rightClickReleased;
    bool leftClickDown;
    bool rightClickDown;
    bool ctrlDown;
    bool goBack;
    bool dropItem;
    bool dropItemAll;

    bool selectSlot1;
    bool selectSlot2;
    bool selectSlot3;
    bool selectSlot4;

    bool testLoseHP;

    // Absorbed from rogues (were hardcoded in main.cpp / debugmode.cpp)
    bool pauseMenu;
    bool debugToggle;
    bool debugToggleEnemy;
    bool debugTogglePlayer;

    float mouseWheel;
};

/**
 * @brief Kelas manajer untuk mengambil dan menguraikan input pemain.
 * Mengabstraksi status tombol/mouse mentah menjadi aksi dan status khusus game.
 */
class PlayerInput
{
public:
    PlayerInput();

    /**
     * @brief Membaca input mentah dari hardware melalui Raylib.
     */
    void PollInput(void);

    /**
     * @brief Mendapatkan snapshot status input mentah.
     */
    const InputState &GetState() const { return Current; }

    // Pemeriksaan boolean untuk status pergerakan/aksi umum
    bool IsMoveUp() const { return Current.moveUp; }
    bool IsMoveDown() const { return Current.moveDown; }
    bool IsMoveLeft() const { return Current.moveLeft; }
    bool IsMoveRight() const { return Current.moveRight; }
    bool IsMoving() const { return Current.moveUp || Current.moveDown || Current.moveLeft || Current.moveRight; }

    bool IsInteract() const { return Current.interact; }
    bool IsRevive() const { return Current.revive; }
    bool IsToggleInventory() const { return Current.toggleInventory; }
    bool IsToggleMap() const { return Current.toggleMap; }
    bool IsLeftClickPressed() const { return Current.leftClickPressed; }
    bool IsRightClickPressed() const { return Current.rightClickPressed; }
    bool IsLeftClickReleased() const { return Current.leftClickReleased; }
    bool IsRightClickReleased() const { return Current.rightClickReleased; }
    bool IsLeftClickDown() const { return Current.leftClickDown; }
    bool IsRightClickDown() const { return Current.rightClickDown; }
    bool IsCtrlDown() { return Current.ctrlDown; }
    bool IsGoBack() const { return Current.goBack; }
    bool IsDropItem() const { return Current.dropItem; }
    bool IsDropItemAll() const { return Current.dropItemAll; }
    bool IsSelectSlot1() const { return Current.selectSlot1; }
    bool IsSelectSlot2() const { return Current.selectSlot2; }
    bool IsSelectSlot3() const { return Current.selectSlot3; }
    bool IsSelectSlot4() const { return Current.selectSlot4; }

    bool IsTestLoseHP() const { return Current.testLoseHP; }

    ItemSlot GetActiveSlot() const { return ActiveSlot; }
    bool IsInventoryOpen() const { return InventoryOpen; }
    bool IsMapOpen() const { return MapOpen; }

    /**
     * @brief Menentukan aksi tingkat tinggi mana yang harus dipicu pada frame ini.
     */
    PlayerAction ResolveAction() const;

    /**
     * @brief Memproses toggle dan logika input persisten.
     */
    void UpdateState(void);

private:
    InputState Current = {};
    ItemSlot ActiveSlot = SLOT_WEAPON_1;
    bool InventoryOpen = false;
    bool MapOpen = false;

    HotbarList doubleLinkedList; ///< Mengelola perputaran hotbar melalui scroll mouse
};

extern PlayerInput InputInstance;
