#pragma once

#include "../lib/raylib/include/raylib.h"
#include "inputLinkedlist.h"

enum PlayerAction
{
    ACTION_NONE,
    ACTION_ATTACK,
    ACTION_DRINK_POTION,
    ACTION_EQUIP_UNEQUIP
};

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
    bool goBack;

    bool selectSlot1;
    bool selectSlot2;
    bool selectSlot3;
    bool selectSlot4;

    bool testLoseHP;

    float mouseWheel;
};

class PlayerInput
{
public:
    PlayerInput();

    void PollInput(void);

    const InputState &GetState() const { return Current; }

    bool IsMoveUp() const { return Current.moveUp; }
    bool IsMoveDown() const { return Current.moveDown; }
    bool IsMoveLeft() const { return Current.moveLeft; }
    bool IsMoveRight() const { return Current.moveRight; }
    bool IsMoving() const { return Current.moveUp || Current.moveDown || Current.moveLeft || Current.moveRight; }

    bool IsInteract()        const { return Current.interact; }
    bool IsRevive()          const { return Current.revive; }
    bool IsToggleInventory() const { return Current.toggleInventory; }
    bool IsToggleMap() const { return Current.toggleMap; }
    bool IsLeftClickPressed() const { return Current.leftClickPressed; }
    bool IsGoBack() const { return Current.goBack; }

    bool IsSelectSlot1() const { return Current.selectSlot1; }
    bool IsSelectSlot2() const { return Current.selectSlot2; }
    bool IsSelectSlot3() const { return Current.selectSlot3; }
    bool IsSelectSlot4() const { return Current.selectSlot4; }

    bool IsTestLoseHP() const { return Current.testLoseHP; }

    ItemSlot GetActiveSlot() const { return ActiveSlot; }
    bool IsInventoryOpen() const { return InventoryOpen; }
    bool IsMapOpen() const { return MapOpen; }

    PlayerAction ResolveAction() const;

    void UpdateState(void);

private:
    InputState Current = {};
    ItemSlot ActiveSlot = SLOT_WEAPON_1;
    bool InventoryOpen = false;
    bool MapOpen = false;

    HotbarList doubleLinkedList;
};

extern PlayerInput InputInstance;