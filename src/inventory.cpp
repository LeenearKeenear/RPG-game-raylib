#include "../include/inventory.h"
#include "../include/player.h"
#include "../include/input.h"
#include "../include/effects.h"

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
        Effects::AddLog("Potion telah habis!");
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

InventoryItem GetActiveHotbarItem(const Player& player) {
    ItemSlot activeSlot = InputInstance.GetActiveSlot();
    int idx = (int)activeSlot - 1;
    if (idx >= 0 && idx < 4) {
        return player.Hotbar[idx];
    }
    return {ITEM_NONE, "", 0, 0, 0, 0, 0};
}

void SetupAttackStats(Player& player, Direction attackFaceDir) {
    ItemSlot activeSlot = InputInstance.GetActiveSlot();
    InventoryItem activeItem = GetActiveHotbarItem(player);
    
    float baseAngle = 0.0f;
    switch (attackFaceDir)
    {
    case RIGHT: baseAngle = 0.0f; break;
    case DOWN:  baseAngle = 90.0f; break;
    case LEFT:  baseAngle = 180.0f; break;
    case UP:    baseAngle = -90.0f; break;
    }
    player.Swing.baseAngle = baseAngle;

    // Logika Offset Senjata (Menyelaraskan sprite senjata dengan tangan/posisi)
    if (activeSlot == SLOT_WEAPON_1)
    {
        switch (attackFaceDir)
        {
        case UP:    player.Swing.center.y -= 8; player.Swing.center.x += 4; break;
        case DOWN:  player.Swing.center.y += 8; player.Swing.center.x -= 4; break;
        case LEFT:  player.Swing.center.x -= 8; player.Swing.center.y -= 4; break;
        case RIGHT: player.Swing.center.x += 8; player.Swing.center.y += 4; break;
        }
    }
    else
    {
        switch (attackFaceDir)
        {
        case UP:    player.Swing.center.y -= 20; break;
        case DOWN:  player.Swing.center.y += 20; break;
        case LEFT:  player.Swing.center.x -= 20; break;
        case RIGHT: player.Swing.center.x += 20; break;
        }
    }

    // Diferensiasi Arketipe Senjata
    if (activeSlot == SLOT_WEAPON_1) { // Pedang (Tusukan/Thrusting)
        player.Swing.type = ATTACK_THRUST;
        player.Swing.duration = 0.25f;
        player.Swing.reach = 40.0f;
        player.Swing.breadth = 16.0f;
        player.Swing.startAngle = baseAngle;
        player.Swing.sweepAngle = 0.0f;
        player.Swing.damage = 15.0f;
        player.Swing.knockbackForce = 0.6f;
    } else { // Senjata Berat (Ayunan/Slashing)
        player.Swing.type = ATTACK_SLASH;
        player.Swing.duration = 0.5f; // Axe: dipercepat ke 0.5s
        player.Swing.reach = 48.0f;
        player.Swing.breadth = 56.0f;
        player.Swing.startAngle = baseAngle + 55.0f;
        player.Swing.sweepAngle = -95.0f;
        player.Swing.damage = 25.0f;
        player.Swing.knockbackForce = 1.8f;
    }
}

float GetAttackManaCost(const Player& player) {
    InventoryItem activeItem = GetActiveHotbarItem(player);
    float manaCost = player.AttackManaCost;
    if (activeItem.iconX == 7) manaCost = 15.0f; // Axe: cost diperbesar ke 15
    return manaCost;
}

}
