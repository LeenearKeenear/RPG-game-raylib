#include "../include/input.h"

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

    Current.moveUp    = IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W);
    Current.moveDown  = IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S);
    Current.moveLeft  = IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A);
    Current.moveRight = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);

    Current.interact        = IsKeyPressed(KEY_E);
    Current.revive          = IsKeyPressed(KEY_R);
    Current.toggleInventory = IsKeyPressed(KEY_I);
    Current.toggleMap       = IsKeyPressed(KEY_M);
    
    Current.leftClickPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    Current.leftClickDown    = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

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
    if (InventoryOpen)
        return ACTION_EQUIP_UNEQUIP;

    switch (ActiveSlot)
    {
    case SLOT_WEAPON_1:
    case SLOT_WEAPON_2:
        return ACTION_ATTACK;

    case SLOT_POTION_1:
    case SLOT_POTION_2:
        return ACTION_DRINK_POTION;

    case SLOT_NONE:
    default:
        return ACTION_ATTACK;
    }
}