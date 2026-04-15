#pragma once

class Player;

// ================================================================
// PlayerInput Class
// Handler component untuk semua input dari player.
// Membaca input keyboard dan mengupdate state player.
// ================================================================
class PlayerInput
{
public:
    static void HandleInput(Player* player);

private:
    static void HandleMovement(Player* player);
    static void HandleHotbar(Player* player);
    static void HandleActionKey(Player* player);
    static void HandleInteract(Player* player);
    static void HandleInventory(Player* player);
    static void HandleMap(Player* player);
    static void HandleTestDeath(Player* player);
    static void HandleTestRevive(Player* player);
};
