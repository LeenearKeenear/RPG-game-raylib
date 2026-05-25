#include "input.h"
#include "player.h"
#include "item.h"

/** @brief Instance global input */
PlayerInput InputInstance;

PlayerInput::PlayerInput()
{
    doubleLinkedList.Initialize();
}

void PlayerInput::PollInput(void)
{
    if (!IsWindowFocused())
    {
        Current = {};
        return;
    }

    Current.moveUp = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
    Current.moveDown = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);
    Current.moveLeft = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
    Current.moveRight = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);

    // --- Actions (KeyPressed — tap sekali / baru diteken) ---
    Current.interact = IsKeyPressed(KEY_E);
    Current.revive = IsKeyPressed(KEY_R);
    Current.toggleInventory = IsKeyPressed(KEY_I);
    Current.toggleMap = IsKeyPressed(KEY_M);
    Current.dropItem = IsKeyPressed(KEY_Q);
    Current.dropItemAll = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

    Current.leftClickPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    Current.rightClickPressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    Current.leftClickReleased = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    Current.rightClickReleased = IsMouseButtonReleased(MOUSE_BUTTON_RIGHT);
    Current.leftClickDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    Current.rightClickDown = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    Current.ctrlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown (KEY_RIGHT_CONTROL);

    Current.selectSlot1 = IsKeyPressed(KEY_ONE);
    Current.selectSlot2 = IsKeyPressed(KEY_TWO);
    Current.selectSlot3 = IsKeyPressed(KEY_THREE);
    Current.selectSlot4 = IsKeyPressed(KEY_FOUR);

    Current.testLoseHP = IsKeyPressed(KEY_K);

    Current.mouseWheel = GetMouseWheelMove();
    Current.goBack = IsKeyPressed(KEY_B);
}

void PlayerInput::UpdateState(void)
{
    if (Current.toggleInventory)
    {
        InventoryOpen = !InventoryOpen;
        if (InventoryOpen)
            MapOpen = false;

        TraceLog(LOG_INFO, "INPUT: Inventory %s", InventoryOpen ? "OPENED" : "CLOSED");
    }

    if (Current.toggleMap)
    {
        MapOpen = !MapOpen;
        if (MapOpen)
            InventoryOpen = false;

        TraceLog(LOG_INFO, "INPUT: Map %s", MapOpen ? "OPENED" : "CLOSED");
    }

    if (Current.selectSlot1)
    {
        ActiveSlot = SLOT_WEAPON_1;
        doubleLinkedList.SetCurrentBySlot(SLOT_WEAPON_1);
        TraceLog(LOG_INFO, "INPUT: Selected SLOT 1 (Weapon 1)");
    }
    if (Current.selectSlot2)
    {
        ActiveSlot = SLOT_WEAPON_2;
        doubleLinkedList.SetCurrentBySlot(SLOT_WEAPON_2);
        TraceLog(LOG_INFO, "INPUT: Selected SLOT 2 (Weapon 2)");
    }
    if (Current.selectSlot3)
    {
        ActiveSlot = SLOT_POTION_1;
        doubleLinkedList.SetCurrentBySlot(SLOT_POTION_1);
        TraceLog(LOG_INFO, "INPUT: Selected SLOT 3 (Potion 1)");
    }
    if (Current.selectSlot4)
    {
        ActiveSlot = SLOT_POTION_2;
        doubleLinkedList.SetCurrentBySlot(SLOT_POTION_2);
        TraceLog(LOG_INFO, "INPUT: Selected SLOT 4 (Potion 2)");
    }

    if (Current.mouseWheel != 0)
    {
        if (Current.mouseWheel > 0)
            ActiveSlot = doubleLinkedList.GetPrev();
        else
            ActiveSlot = doubleLinkedList.GetNext();

        TraceLog(LOG_INFO, "INPUT: Mouse Wheel moved, selected slot %d", (int)ActiveSlot);
    }

    if (Current.interact)
    {
        TraceLog(LOG_INFO, "INPUT: Interact (E) pressed");
    }
}

PlayerAction PlayerInput::ResolveAction() const
{

    if (Current.dropItem)
        return ACTION_DROP_ITEM;

    // Ambil item di slot aktif
    int slotIdx = (int)ActiveSlot - 1;
    if (slotIdx < 0 || slotIdx >= 4)
        return ACTION_NONE;

    const InventoryItem &item = PlayerInstance.GetHotbarItem(slotIdx);
    if (item.definitionId == -1)
        return ACTION_NONE;

    const ItemDefinition &def = itemDefs.GetById(item.definitionId);
    if (def.category == ITEM_WEAPON)
        return ACTION_ATTACK;
    if (def.category == ITEM_POTION)
        return ACTION_DRINK_POTION;

    return ACTION_NONE;
}
