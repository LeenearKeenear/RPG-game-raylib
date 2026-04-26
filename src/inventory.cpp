#include "../include/inventory.h"
#include "../include/player.h"
#include "../include/input.h"

namespace Inventory {

void HandleInventoryActions(Player& player) {
    if (!InputInstance.IsLeftClickPressed()) return;

    PlayerAction action = InputInstance.ResolveAction();

    if (action == ACTION_DRINK_POTION) {
        int slotIdx = (int)InputInstance.GetActiveSlot() - 1;
        if (slotIdx >= 0 && slotIdx < 4) {
            UsePotion(player, slotIdx);
        }
    } else if (action == ACTION_EQUIP_UNEQUIP) {
        TraceLog(LOG_INFO, "PLAYER: Equip/Unequip from inventory!");
    }
}

void UsePotion(Player& player, int slotIndex) {
    InventoryItem& item = player.Hotbar[slotIndex];

    if (item.type != ITEM_POTION || item.amount <= 0) {
        TraceLog(LOG_INFO, "PLAYER: No potion in slot %d!", slotIndex + 1);
        return;
    }

    if (item.name.find("Mana") != std::string::npos) {
        player.Mana += item.healValue;
        if (player.Mana > player.MaxMana) player.Mana = player.MaxMana;
        TraceLog(LOG_INFO, "PLAYER: Drink potion! %s (slot %d) - Healed Mana by %d! Current: %.1f", 
                 item.name.c_str(), slotIndex + 1, item.healValue, player.Mana);
    } else {
        player.Health += item.healValue;
        if (player.Health > player.MaxHealth) player.Health = player.MaxHealth;
        TraceLog(LOG_INFO, "PLAYER: Drink potion! %s (slot %d) - Healed Health by %d! Current: %.1f", 
                 item.name.c_str(), slotIndex + 1, item.healValue, player.Health);
    }

    item.amount--;
    if (item.amount <= 0) {
        item = {ITEM_NONE, "", 0, 0, 0, 0, 0};
    }
}

bool AddToInventory(Player& player, const Item& item) {
    // Cari slot kosong di Hotbar dulu
    for (int i = 0; i < 4; i++) {
        if (player.Hotbar[i].type == ITEM_NONE) {
            player.Hotbar[i].type = item.category;
            player.Hotbar[i].name = item.name;
            player.Hotbar[i].amount = 1;
            
            // Set icon berdasarkan kategori (sama dengan item.cpp)
            if (item.category == ITEM_POTION) {
                player.Hotbar[i].iconX = 7;
                player.Hotbar[i].iconY = 8;
                player.Hotbar[i].healValue = 20; // Default heal value
            } else if (item.category == ITEM_WEAPON) {
                player.Hotbar[i].iconX = 6;
                player.Hotbar[i].iconY = 4;
                player.Hotbar[i].damage = 10; // Default damage
            }
            return true;
        }
    }

    // Jika Hotbar penuh, cari di Bag
    for (int i = 0; i < 49; i++) {
        if (player.Bag[i].type == ITEM_NONE) {
            player.Bag[i].type = item.category;
            player.Bag[i].name = item.name;
            player.Bag[i].amount = 1;
            
            if (item.category == ITEM_POTION) {
                player.Bag[i].iconX = 7;
                player.Bag[i].iconY = 8;
                player.Bag[i].healValue = 20;
            } else if (item.category == ITEM_WEAPON) {
                player.Bag[i].iconX = 6;
                player.Bag[i].iconY = 4;
                player.Bag[i].damage = 10;
            }
            return true;
        }
    }

    return false; // Inventory penuh
}

}
