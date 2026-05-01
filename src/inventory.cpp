#include "../include/item.h"
#include "../include/inventory.h"
#include "../include/player.h"
#include "../include/input.h"
#include "../include/effects.h"

namespace Inventory
{

    void HandleInventoryActions(Player &player)
    {
        if (!InputInstance.IsLeftClickPressed())
            return;

        PlayerAction action = InputInstance.ResolveAction();

        if (action == ACTION_DRINK_POTION)
        {
            int slotIdx = (int)InputInstance.GetActiveSlot() - 1;
            if (slotIdx >= 0 && slotIdx < 4)
                UsePotion(player, slotIdx);
        }
        else if (action == ACTION_EQUIP_UNEQUIP)
        {
            TraceLog(LOG_INFO, "PLAYER: Equip/Unequip from inventory!");
        }
    }

    void UsePotion(Player &player, int slotIndex)
    {
        InventoryItem &slot = player.Hotbar[slotIndex];
        if (slot.definitionId == -1 || slot.amount <= 0)
        {
            Effects::AddLog("Potion telah habis!");
            return;
        }

        const ItemDefinition &def = itemDefs.Get(slot.definitionId);
        if (def.category != ITEM_POTION)
            return;

        const PotionData &potion = std::get<PotionData>(def.data);
        if (potion.isMana)
            player.Mana = std::min(player.Mana + (float)potion.healValue, player.MaxMana);
        else
            player.Health = std::min(player.Health + (float)potion.healValue, player.MaxHealth);

        slot.amount--;
        if (slot.amount <= 0)
            slot = {-1, 0};
    }

    bool AddToInventory(Player &player, const ItemSpawn &item)
    {
        const ItemDefinition &def = itemDefs.Get(item.definitionId);
        bool isStackable = (def.category != ITEM_WEAPON);

        // Coba merge ke slot yang sudah ada item sama
        if (isStackable)
        {
            // Cek hotbar dulu
            for (int i = 0; i < 4; i++)
            {
                if (player.Hotbar[i].definitionId == item.definitionId && player.Hotbar[i].amount < 8)
                {
                    player.Hotbar[i].amount++;
                    return true;
                }
            }
            // Cek bag
            for (int i = 0; i < 49; i++)
            {
                if (player.Bag[i].definitionId == item.definitionId && player.Bag[i].amount < 8)
                {
                    player.Bag[i].amount++;
                    return true;
                }
            }
        }

        // Slot kosong di hotbar
        for (int i = 0; i < 4; i++)
        {
            if (player.Hotbar[i].definitionId == -1)
            {
                player.Hotbar[i] = {item.definitionId, 1};
                return true;
            }
        }

        // Slot kosong di bag
        for (int i = 0; i < 49; i++)
        {
            if (player.Bag[i].definitionId == -1)
            {
                player.Bag[i] = {item.definitionId, 1};
                return true;
            }
        }

        return false;
    }

    InventoryItem GetActiveHotbarItem(const Player &player)
    {
        int idx = (int)InputInstance.GetActiveSlot() - 1;
        if (idx >= 0 && idx < 4)
            return player.Hotbar[idx];
        return {-1, 0};
    }

    void SetupAttackStats(Player &player, Direction attackFaceDir)
    {
        InventoryItem activeItem = GetActiveHotbarItem(player);
        if (activeItem.definitionId == -1)
            return;

        const ItemDefinition &def = itemDefs.Get(activeItem.definitionId);
        if (def.category != ITEM_WEAPON)
            return;

        const WeaponData &wpn = std::get<WeaponData>(def.data);

        float baseAngle = 0.0f;
        switch (attackFaceDir)
        {
        case RIGHT:
            baseAngle = 0.0f;
            break;
        case DOWN:
            baseAngle = 90.0f;
            break;
        case LEFT:
            baseAngle = 180.0f;
            break;
        case UP:
            baseAngle = -90.0f;
            break;
        }

        player.Swing.baseAngle = baseAngle;

        switch (attackFaceDir)
        {
        case UP:
            player.Swing.center.y -= wpn.centerOffset.y;
            player.Swing.center.x += wpn.centerOffset.x;
            break;
        case DOWN:
            player.Swing.center.y += wpn.centerOffset.y;
            player.Swing.center.x -= wpn.centerOffset.x;
            break;
        case LEFT:
            player.Swing.center.x -= wpn.centerOffset.x;
            player.Swing.center.y -= wpn.centerOffset.y;
            break;
        case RIGHT:
            player.Swing.center.x += wpn.centerOffset.x;
            player.Swing.center.y += wpn.centerOffset.y;
            break;
        }

        player.Swing.type = wpn.attackType;
        player.Swing.duration = wpn.duration;
        player.Swing.reach = wpn.reach;
        player.Swing.breadth = wpn.breadth;
        player.Swing.startAngle = baseAngle + wpn.startAngleOffset;
        player.Swing.sweepAngle = wpn.sweepAngle;
        player.Swing.damage = wpn.damage;
        player.Swing.knockbackForce = wpn.knockbackForce;
    }

    float GetAttackManaCost(const Player &player)
    {
        InventoryItem activeItem = GetActiveHotbarItem(player);
        if (activeItem.definitionId == -1)
            return player.AttackManaCost;

        const ItemDefinition &def = itemDefs.Get(activeItem.definitionId);
        if (def.category != ITEM_WEAPON)
            return player.AttackManaCost;

        return std::get<WeaponData>(def.data).manaCost;
    }
}