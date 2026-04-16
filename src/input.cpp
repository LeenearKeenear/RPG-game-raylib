// ================================================================
// Input System — Implementation
// Centralized input polling dan state management.
//
// PollInput() dibaca sekali per frame di awal Player::Update()
// lalu hasilnya dipakai di seluruh logic player.
// ================================================================

#include "../include/input.h"

// ================================================================
// Global
// ================================================================

// global instance input — diakses file lain via extern
PlayerInput InputInstance;

// ================================================================
// PollInput()
// Baca semua key binding dalam satu pass.
// Movement pakai IsKeyDown (hold), sisanya IsKeyPressed (tap).
// ================================================================
void PlayerInput::PollInput(void)
{
    // kalau window tidak focused, clear input biar gak nyangkut
    if (!IsWindowFocused())
    {
        Current = {};
        return;
    }

    // --- Movement (KeyDown — hold) ---
    Current.moveUp    = IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W);
    Current.moveDown  = IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S);
    Current.moveLeft  = IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A);
    Current.moveRight = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);

    // --- Actions (KeyPressed — tap sekali) ---
    Current.interact        = IsKeyPressed(KEY_E);
    Current.revive          = IsKeyPressed(KEY_R);
    Current.toggleInventory = IsKeyPressed(KEY_I);
    Current.toggleMap       = IsKeyPressed(KEY_M);
    
    // Gunakan deteksi manual karena IsMouseButtonPressed mungkin 'dicuri' by main.cpp/pauseMenu.cpp
    static bool mouseWasDown = false;
    bool mouseIsDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    Current.leftClickPressed = mouseIsDown && !mouseWasDown;
    mouseWasDown = mouseIsDown;

    // --- Slot Selection (KeyPressed — tap sekali) ---
    Current.selectSlot1 = IsKeyPressed(KEY_ONE);
    Current.selectSlot2 = IsKeyPressed(KEY_TWO);
    Current.selectSlot3 = IsKeyPressed(KEY_THREE);
    Current.selectSlot4 = IsKeyPressed(KEY_FOUR);

    // --- Test / Debug (KeyPressed — tap sekali) ---
    Current.testLoseHP = IsKeyPressed(KEY_K);
    Current.testLoseMP = IsKeyPressed(KEY_J);
}

// ================================================================
// UpdateState()
// Update internal state berdasarkan input yang baru di-poll.
// Handle toggle UI (inventori/map) dan slot selection.
// Panggil setelah PollInput().
// ================================================================
void PlayerInput::UpdateState(void)
{
    // --- Toggle Inventori ---
    if (Current.toggleInventory)
    {
        InventoryOpen = !InventoryOpen;
        // kalau buka inventori, tutup map
        if (InventoryOpen)
            MapOpen = false;

        TraceLog(LOG_INFO, "INPUT: Inventory %s", InventoryOpen ? "OPENED" : "CLOSED");
    }

    // --- Toggle Map ---
    if (Current.toggleMap)
    {
        MapOpen = !MapOpen;
        // kalau buka map, tutup inventori
        if (MapOpen)
            InventoryOpen = false;

        TraceLog(LOG_INFO, "INPUT: Map %s", MapOpen ? "OPENED" : "CLOSED");
    }

    // --- Slot Selection ---
    if (Current.selectSlot1)
    {
        ActiveSlot = SLOT_WEAPON_1;
        TraceLog(LOG_INFO, "INPUT: Selected SLOT 1 (Weapon 1)");
    }
    if (Current.selectSlot2)
    {
        ActiveSlot = SLOT_WEAPON_2;
        TraceLog(LOG_INFO, "INPUT: Selected SLOT 2 (Weapon 2)");
    }
    if (Current.selectSlot3)
    {
        ActiveSlot = SLOT_POTION_1;
        TraceLog(LOG_INFO, "INPUT: Selected SLOT 3 (Potion 1)");
    }
    if (Current.selectSlot4)
    {
        ActiveSlot = SLOT_POTION_2;
        TraceLog(LOG_INFO, "INPUT: Selected SLOT 4 (Potion 2)");
    }

    // --- Interact ---
    if (Current.interact)
    {
        TraceLog(LOG_INFO, "INPUT: Interact (E) pressed");
    }
}

// ================================================================
// ResolveSpaceAction()
// Tentukan apa yang terjadi saat left click berdasarkan context:
// 1. Inventori terbuka → equip/unequip
// 2. Slot senjata aktif (1/2) → attack
// 3. Slot potion aktif (3/4) → minum potion
// 4. Selain itu → none (atau default attack)
// ================================================================
SpaceAction PlayerInput::ResolveSpaceAction() const
{
    // prioritas 1: kalau inventori terbuka → equip/unequip
    if (InventoryOpen)
        return ACTION_EQUIP_UNEQUIP;

    // prioritas 2: berdasarkan slot yang aktif
    switch (ActiveSlot)
    {
    case SLOT_WEAPON_1:
    case SLOT_WEAPON_2:
        return ACTION_ATTACK;

    case SLOT_POTION_1:
    case SLOT_POTION_2:
        return ACTION_DRINK_POTION;

    case SLOT_NONE:
    default:
        // default: kalau belum pilih slot apapun, treat as attack
        return ACTION_ATTACK;
    }
}
