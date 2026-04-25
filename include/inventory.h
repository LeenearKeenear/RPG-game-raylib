#pragma once
#include <string>
#include "item.h"


struct InventoryItem {
    ItemCategory type = ITEM_NONE;
    std::string name = "";
    int amount = 0;
    int damage = 0;
    int healValue = 0;
    int iconX = 0;
    int iconY = 0;
};
