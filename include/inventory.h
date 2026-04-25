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
    int iconX = 0;
    int iconY = 0;
};

class Player;

namespace Inventory {
    void HandleInventoryActions(Player& player);
    void UsePotion(Player& player, int slotIndex);
}
