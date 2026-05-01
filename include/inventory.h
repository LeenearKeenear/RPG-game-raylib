#pragma once
#include "item.h"
#include "animation.h"

class Player;

namespace Inventory {
    void HandleInventoryActions(Player& player);
    void UsePotion(Player& player, int slotIndex);
    bool AddToInventory(Player& player, const ItemSpawn& item);

    InventoryItem GetActiveHotbarItem(const Player& player);
    void SetupAttackStats(Player& player, Direction attackFaceDir);
    float GetAttackManaCost(const Player& player);
}