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

}
