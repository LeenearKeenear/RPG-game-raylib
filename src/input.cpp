/**
 * @file input.cpp
 * @brief Implementasi dari Input System & Key Binding Module
 * 
 * Implementasi dari class PlayerInput yang dideklarasikan di input.h
 * Handle polling input, state management, dan resolusi aksi left click.
 */

// ================================================================
// Input System — Implementation
// Centralized input polling dan state management.
//
// PollInput() dibaca sekali per frame di awal Player::Update()
// lalu hasilnya dipakai di seluruh logic player.
// ================================================================

#include "../include/input.h"

/*==============================================================================
 * Global Variables
 *==============================================================================*/

// ================================================================
// Global
// ================================================================

/** Global instance input — diakses file lain via extern */
PlayerInput InputInstance;

/*==============================================================================
 * Public Methods - Polling & Update
 *==============================================================================*/

// ================================================================
// PollInput()
// Baca semua key binding dalam satu pass.
// Movement pakai IsKeyDown (hold), sisanya IsKeyPressed (tap).
// ================================================================
void PlayerInput::PollInput(void)
{
    // --- Movement (KeyDown — hold / ditahan) ---
    // Arrow keys dan WASD support
    Current.moveUp    = IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W);
    Current.moveDown  = IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S);
    Current.moveLeft  = IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A);
    Current.moveRight = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);

    // --- Actions (KeyPressed — tap sekali / baru diteken) ---
    Current.interact        = IsKeyPressed(KEY_E);
    Current.kill            = IsKeyPressed(KEY_K);        // debug: langsung matiin player
    Current.revive          = IsKeyPressed(KEY_R);        // debug: revive player
    Current.toggleInventory = IsKeyPressed(KEY_I);
    Current.toggleMap       = IsKeyPressed(KEY_M);
    Current.leftClickPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    Current.goBack = IsKeyPressed(KEY_B);                 // tombol B buat kembali ke map sebelumnya

    // --- Slot Selection (KeyPressed — tap sekali) ---
    Current.selectSlot1 = IsKeyPressed(KEY_ONE);   // key 1
    Current.selectSlot2 = IsKeyPressed(KEY_TWO);   // key 2
    Current.selectSlot3 = IsKeyPressed(KEY_THREE); // key 3
    Current.selectSlot4 = IsKeyPressed(KEY_FOUR);  // key 4
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
        // kalau buka inventori, tutup map (biar gak bentrok)
        if (InventoryOpen)
            MapOpen = false;

        TraceLog(LOG_INFO, "INPUT: Inventory %s", InventoryOpen ? "OPENED" : "CLOSED");
    }

    // --- Toggle Map ---
    if (Current.toggleMap)
    {
        MapOpen = !MapOpen;
        // kalau buka map, tutup inventori (biar gak bentrok)
        if (MapOpen)
            InventoryOpen = false;

        TraceLog(LOG_INFO, "INPUT: Map %s", MapOpen ? "OPENED" : "CLOSED");
    }

    // --- Slot Selection ---
    // Update active slot berdasarkan key yang ditekan
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

    // --- Interact (logging buat debugging) ---
    if (Current.interact)
    {
        TraceLog(LOG_INFO, "INPUT: Interact (E) pressed");
    }
}

/*==============================================================================
 * Public Methods - Action Resolution
 *==============================================================================*/

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
        return ACTION_ATTACK;       // slot senjata → attack

    case SLOT_POTION_1:
    case SLOT_POTION_2:
        return ACTION_DRINK_POTION; // slot potion → minum potion

    case SLOT_NONE:
    default:
        // default: kalau belum pilih slot apapun, treat as attack
        return ACTION_ATTACK;
    }
}