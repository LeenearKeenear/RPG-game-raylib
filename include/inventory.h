#pragma once
#include <string>

enum ItemType {
    ITEM_NONE,
    ITEM_WEAPON,
    ITEM_POTION
};

struct InventoryItem {
    ItemType type = ITEM_NONE;
    std::string name = "";
    int amount = 0;
    int damage = 0;
    int healValue = 0;
};
