#include "item.h"
#include "inventory.h"
#include "player.h"
#include "input.h"
#include "effects.h"

namespace Inventory
{
    /*==============================================================================
     * Utilitas Inventory
     *==============================================================================*/

    bool HasInventorySpace(const Player &player)
    {
        for (int i = 0; i < player.GetMaxHotbar(); i++)
            if (player.GetHotbarItem(i).definitionId == -1)
                return true;
        for (int i = 0; i < player.GetMaxBag(); i++)
            if (player.GetBagItem(i).definitionId == -1)
                return true;
        return false;
    }

    InventoryItem GetActiveHotbarItem(const Player &player)
    {
        int idx = (int)InputInstance.GetActiveSlot() - 1;
        if (idx >= 0 && idx < player.GetMaxHotbar())
            return player.GetHotbarItem(idx);
        return {-1, 0};
    }

    /*==============================================================================
     * Input & Aksi Inventory
     *==============================================================================*/

    void HandleInventoryActions(Player &player)
    {
        // Inventory terbuka = block semua aksi shortcut
        if (InputInstance.IsInventoryOpen())
            return;

        if (player.IsDashing)
            return;

        if (!InputInstance.IsRightClickPressed())
            return;

        PlayerAction action = InputInstance.ResolveAction();

        if (action == ACTION_DRINK_POTION)
        {
            int slotIdx = (int)InputInstance.GetActiveSlot() - 1;
            if (slotIdx >= 0 && slotIdx < player.GetMaxHotbar())
                UsePotion(player, slotIdx);
        }
        else if (action == ACTION_EQUIP_UNEQUIP)
        {
            TraceLog(LOG_INFO, "PLAYER: Equip/Unequip from inventory!");
        }
    }

    /*==============================================================================
     * Potion
     *==============================================================================*/

    void UsePotion(Player &player, int slotIndex)
    {
        InventoryItem &slot = player.GetHotbarItem(slotIndex);
        if (slot.definitionId == -1 || slot.amount <= 0)
        {
            Effects::AddLog("Potion telah habis!");
            return;
        }

        const ItemDefinition &def = itemDefs.GetById(slot.definitionId);
        if (def.category != ITEM_POTION)
            return;

        const PotionData &potion = std::get<PotionData>(def.data);

        // Jangan konsumsi jika stat sudah penuh
        if (potion.isMana && player.Mana >= player.MaxMana)
        {
            Effects::AddLog("Mana sudah penuh!");
            return;
        }
        if (!potion.isMana && player.Health >= player.MaxHealth)
        {
            Effects::AddLog("Health sudah penuh!");
            return;
        }

        if (potion.isMana)
            player.Mana = std::min(player.Mana + (float)potion.healValue, player.MaxMana);
        else
            player.Health = std::min(player.Health + (float)potion.healValue, player.MaxHealth);

        slot.amount--;
        if (slot.amount <= 0)
            slot = {-1, 0}; // kosongkan slot jika habis
    }

    /*==============================================================================
     * Penambahan Item ke Inventory
     *==============================================================================*/

    bool AddToInventory(Player &player, const ItemSpawn &item)
    {
        const ItemDefinition &def = itemDefs.GetById(item.definitionId);
        bool isStackable = def.isStackable;
        int maxStack = def.maxStack;
        int remaining = item.amount;
        int activeSlot = (int)InputInstance.GetActiveSlot() - 1;

        // 1. Prioritaskan slot hotbar aktif
        if (activeSlot >= 0 && activeSlot < player.GetMaxHotbar())
        {
            InventoryItem &active = player.GetHotbarItem(activeSlot);
            if (active.definitionId == -1)
            {
                int add = std::min(remaining, maxStack);
                active = {item.definitionId, add};
                remaining -= add;
            }
            else if (isStackable && active.definitionId == item.definitionId && active.amount < maxStack)
            {
                int space = maxStack - active.amount;
                int add = std::min(remaining, space);
                active.amount += add;
                remaining -= add;
            }
        }

        if (remaining <= 0)
            return true;

        // 2. Merge ke stack yang sudah ada di hotbar & bag
        if (isStackable)
        {
            for (int i = 0; i < player.GetMaxHotbar() && remaining > 0; i++)
            {
                if (i == activeSlot)
                    continue;
                if (player.GetHotbarItem(i).definitionId == item.definitionId && player.GetHotbarItem(i).amount < maxStack)
                {
                    int space = maxStack - player.GetHotbarItem(i).amount;
                    int add = std::min(remaining, space);
                    player.GetHotbarItem(i).amount += add;
                    remaining -= add;
                }
            }
            for (int i = 0; i < player.GetMaxBag() && remaining > 0; i++)
            {
                if (player.GetBagItem(i).definitionId == item.definitionId && player.GetBagItem(i).amount < maxStack)
                {
                    int space = maxStack - player.GetBagItem(i).amount;
                    int add = std::min(remaining, space);
                    player.GetBagItem(i).amount += add;
                    remaining -= add;
                }
            }
        }

        if (remaining <= 0)
            return true;

        // 3. Slot kosong hotbar (selain active slot)
        for (int i = 0; i < player.GetMaxHotbar() && remaining > 0; i++)
        {
            if (i == activeSlot)
                continue;
            if (player.GetHotbarItem(i).definitionId == -1)
            {
                int add = std::min(remaining, maxStack);
                player.GetHotbarItem(i) = {item.definitionId, add};
                remaining -= add;
            }
        }

        // 4. Slot kosong bag sebagai fallback terakhir
        for (int i = 0; i < player.GetMaxBag() && remaining > 0; i++)
        {
            if (player.GetBagItem(i).definitionId == -1)
            {
                int add = std::min(remaining, maxStack);
                player.GetBagItem(i) = {item.definitionId, add};
                remaining -= add;
            }
        }

        // true jika minimal sebagian item berhasil masuk
        return remaining < item.amount;
    }

    /*==============================================================================
     * Senjata & Serangan
     *==============================================================================*/

    void SetupAttackStats(Player &player, Direction attackFaceDir)
    {
        InventoryItem activeItem = GetActiveHotbarItem(player);
        if (activeItem.definitionId == -1)
            return;

        const ItemDefinition &def = itemDefs.GetById(activeItem.definitionId);
        if (def.category != ITEM_WEAPON)
            return;

        const WeaponData &wpn = std::get<WeaponData>(def.data);

        // Base angle menentukan orientasi awal swing
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

        // Center offset disesuaikan per arah agar hitbox tidak miring
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

        const ItemDefinition &def = itemDefs.GetById(activeItem.definitionId);
        if (def.category != ITEM_WEAPON)
            return player.AttackManaCost;

        return std::get<WeaponData>(def.data).manaCost;
    }
}
