#include "../include/item.h"
#include "../include/inventory.h"
#include "../include/player.h"
#include "../include/input.h"
#include "../include/effects.h"

namespace Inventory
{
    // cek apakah masih ada space di inventory
    bool HasInventorySpace(const Player &player)
    {
        for (int i = 0; i < PlayerInstance.MaxHotbar; i++)
            if (player.Hotbar[i].definitionId == -1)
                return true;
        for (int i = 0; i < PlayerInstance.MaxBag; i++)
            if (player.Bag[i].definitionId == -1)
                return true;
        return false;
    }

    void HandleInventoryActions(Player &player)
    {
        if (InputInstance.IsInventoryOpen())
            return;

        if (!InputInstance.IsRightClickPressed())
            return;

        PlayerAction action = InputInstance.ResolveAction();

        if (action == ACTION_DRINK_POTION)
        {
            int slotIdx = (int)InputInstance.GetActiveSlot() - 1;
            if (slotIdx >= 0 && slotIdx < PlayerInstance.MaxHotbar)
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
        int remaining = item.amount;
        int activeSlot = (int)InputInstance.GetActiveSlot() - 1;

        // 1. Hotbar aktif dulu
        if (activeSlot >= 0 && activeSlot < PlayerInstance.MaxHotbar)
        {
            InventoryItem &active = player.Hotbar[activeSlot];
            if (active.definitionId == -1)
            {
                int add = std::min(remaining, 8);
                active = {item.definitionId, add};
                remaining -= add;
            }
            else if (isStackable && active.definitionId == item.definitionId && active.amount < 8)
            {
                int space = 8 - active.amount;
                int add = std::min(remaining, space);
                active.amount += add;
                remaining -= add;
            }
        }

        if (remaining <= 0)
            return true;

        // 2. Merge ke slot lain yang sudah ada
        if (isStackable)
        {
            for (int i = 0; i < PlayerInstance.MaxHotbar && remaining > 0; i++)
            {
                if (i == activeSlot)
                    continue;
                if (player.Hotbar[i].definitionId == item.definitionId && player.Hotbar[i].amount < 8)
                {
                    int space = 8 - player.Hotbar[i].amount;
                    int add = std::min(remaining, space);
                    player.Hotbar[i].amount += add;
                    remaining -= add;
                }
            }
            for (int i = 0; i < PlayerInstance.MaxBag && remaining > 0; i++)
            {
                if (player.Bag[i].definitionId == item.definitionId && player.Bag[i].amount < 8)
                {
                    int space = 8 - player.Bag[i].amount;
                    int add = std::min(remaining, space);
                    player.Bag[i].amount += add;
                    remaining -= add;
                }
            }
        }

        if (remaining <= 0)
            return true;

        // 3. Slot kosong hotbar lain
        for (int i = 0; i < PlayerInstance.MaxHotbar && remaining > 0; i++)
        {
            if (i == activeSlot)
                continue;
            if (player.Hotbar[i].definitionId == -1)
            {
                int add = std::min(remaining, 8);
                player.Hotbar[i] = {item.definitionId, add};
                remaining -= add;
            }
        }

        // 4. Bag
        for (int i = 0; i < PlayerInstance.MaxBag && remaining > 0; i++)
        {
            if (player.Bag[i].definitionId == -1)
            {
                int add = std::min(remaining, 8);
                player.Bag[i] = {item.definitionId, add};
                remaining -= add;
            }
        }

        return remaining < item.amount;
    }

    InventoryItem GetActiveHotbarItem(const Player &player)
    {
        int idx = (int)InputInstance.GetActiveSlot() - 1;
        if (idx >= 0 && idx < PlayerInstance.MaxHotbar)
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