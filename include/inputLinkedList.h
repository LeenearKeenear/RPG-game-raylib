#pragma once

enum ItemSlot
{
    SLOT_NONE = 0,
    SLOT_WEAPON_1 = 1,
    SLOT_WEAPON_2 = 2,
    SLOT_POTION_1 = 3,
    SLOT_POTION_2 = 4 
};

struct SlotNode
{
    ItemSlot slot;
    SlotNode* next;
    SlotNode* prev;
};

class HotbarList
{
public:
    HotbarList();
    ~HotbarList();

    void Initialize();
    ItemSlot GetNext();
    ItemSlot GetPrev();
    void SetCurrentBySlot(ItemSlot slot);
    ItemSlot GetCurrentSlot() const { return currentNode ? currentNode->slot : SLOT_NONE; }

private:
    SlotNode* head;
    SlotNode* currentNode;
    SlotNode* CreateNode(ItemSlot slot);
};
