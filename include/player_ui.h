#pragma once

class Player;

// ================================================================
// PlayerUI Class
// Handler component untuk me-render UI overlay player:
// Hotbar, Inventory, Map, dan Death overlay.
// ================================================================
class PlayerUI
{
public:
    static void RenderHUD(Player* player);

private:
    static void RenderInventoryUI(Player* player);
    static void RenderMapUI(Player* player);
    static void RenderDeathOverlay(Player* player);
};
