#include "../include/player_input.h"
#include "../include/player.h"
#include <raylib.h>
#include <cmath>

void PlayerInput::HandleInput(Player* player)
{
    // kalau mati, hanya bisa revive
    if (!player->bIsAlive)
    {
        HandleTestRevive(player);
        return;
    }

    // kalau sedang dalam aksi (attack/potion), tunggu selesai
    if (player->State == PLAYER_ATTACKING || player->State == PLAYER_DRINKING_POTION)
    {
        player->ActionTimer -= GetFrameTime();
        if (player->ActionTimer <= 0.0f)
        {
            player->State = PLAYER_IDLE;
            player->currentState = IDLE;
            player->frame = 0;
            player->frameTime = 0.0f;
            player->isAttacking = false;
            player->ActionTimer = 0.0f;
            TraceLog(LOG_INFO, "Player: Action finished, back to IDLE");
        }
        return; 
    }

    HandleInventory(player);
    HandleMap(player);

    if (player->bInventoryOpen)
    {
        HandleActionKey(player);
        return;            
    }

    if (player->bMapOpen)
        return;

    HandleMovement(player);
    HandleHotbar(player);
    HandleActionKey(player);
    HandleInteract(player);
    HandleTestDeath(player);
    HandleTestRevive(player);
}

void PlayerInput::HandleMovement(Player* player)
{
    player->Velocity = {0, 0};

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        player->Velocity.y -= 1;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        player->Velocity.y += 1;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        player->Velocity.x -= 1;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        player->Velocity.x += 1;

    float Length = sqrtf(player->Velocity.x * player->Velocity.x + player->Velocity.y * player->Velocity.y);
    if (Length != 0)
    {
        player->Velocity.x /= Length;
        player->Velocity.y /= Length;
        if (player->currentState != WALK) {
            player->currentState = WALK;
            player->frame = 0;
            player->frameTime = 0.0f;
        }
        player->State = PLAYER_MOVING;
    }
    else
    {
        if (player->currentState != IDLE) {
            player->currentState = IDLE;
            player->frame = 0;
            player->frameTime = 0.0f;
        }
        if (player->State == PLAYER_MOVING)
            player->State = PLAYER_IDLE;
    }

    if (player->Velocity.x < 0) player->currentDir = LEFT;
    else if (player->Velocity.x > 0) player->currentDir = RIGHT;
    else if (player->Velocity.y < 0) player->currentDir = UP;
    else if (player->Velocity.y > 0) player->currentDir = DOWN;
}

void PlayerInput::HandleHotbar(Player* player)
{
    if (IsKeyPressed(KEY_ONE))
    {
        player->SelectedHotbarSlot = 0;
        TraceLog(LOG_INFO, "Player: Selected slot 1 — %s [%s]",
                 player->Hotbar[0].name, player->Hotbar[0].type == SLOT_WEAPON ? "WEAPON" : "POTION");
    }
    if (IsKeyPressed(KEY_TWO))
    {
        player->SelectedHotbarSlot = 1;
        TraceLog(LOG_INFO, "Player: Selected slot 2 — %s [%s]",
                 player->Hotbar[1].name, player->Hotbar[1].type == SLOT_WEAPON ? "WEAPON" : "POTION");
    }
    if (IsKeyPressed(KEY_THREE))
    {
        player->SelectedHotbarSlot = 2;
        TraceLog(LOG_INFO, "Player: Selected slot 3 — %s [%s]",
                 player->Hotbar[2].name, player->Hotbar[2].type == SLOT_WEAPON ? "WEAPON" : "POTION");
    }
    if (IsKeyPressed(KEY_FOUR))
    {
        player->SelectedHotbarSlot = 3;
        TraceLog(LOG_INFO, "Player: Selected slot 4 — %s [%s]",
                 player->Hotbar[3].name, player->Hotbar[3].type == SLOT_WEAPON ? "WEAPON" : "POTION");
    }
}

void PlayerInput::HandleActionKey(Player* player)
{
    if (!IsKeyPressed(KEY_SPACE))
        return;

    if (player->bInventoryOpen)
    {
        TraceLog(LOG_INFO, "Player: [SPACE] Equip/Unequip item in inventory");
        return;
    }

    HotbarSlot &currentSlot = player->Hotbar[player->SelectedHotbarSlot];
    if (currentSlot.type == SLOT_WEAPON)
    {
        player->State = PLAYER_ATTACKING;
        player->ActionTimer = 0.25f; // Durasi attack disesuaikan 2 frame x 0.12f

        player->currentState = ATTACK;
        player->frame = 0;
        player->frameTime = 0.0f;
        player->isAttacking = true;

        TraceLog(LOG_INFO, "Player: [SPACE] ATTACK with %s!", currentSlot.name);
        return;
    }

    if (currentSlot.type == SLOT_POTION)
    {
        player->State = PLAYER_DRINKING_POTION;
        player->ActionTimer = player->ActionDuration;
        TraceLog(LOG_INFO, "Player: [SPACE] DRINK %s!", currentSlot.name);
        return;
    }
}

void PlayerInput::HandleInteract(Player* player)
{
    if (!IsKeyPressed(KEY_E))
        return;

    player->State = PLAYER_INTERACTING;
    TraceLog(LOG_INFO, "Player: [E] Interact at position (%.1f, %.1f)", player->Position.x, player->Position.y);
    player->State = PLAYER_IDLE;
}

void PlayerInput::HandleInventory(Player* player)
{
    if (!IsKeyPressed(KEY_I))
        return;

    player->bInventoryOpen = !player->bInventoryOpen;

    if (player->bInventoryOpen)
    {
        player->bMapOpen = false;
        TraceLog(LOG_INFO, "Player: [I] Inventory OPENED");
    }
    else
    {
        TraceLog(LOG_INFO, "Player: [I] Inventory CLOSED");
    }
}

void PlayerInput::HandleMap(Player* player)
{
    if (!IsKeyPressed(KEY_M))
        return;

    player->bMapOpen = !player->bMapOpen;

    if (player->bMapOpen)
    {
        player->bInventoryOpen = false;
        TraceLog(LOG_INFO, "Player: [M] Map OPENED");
    }
    else
    {
        TraceLog(LOG_INFO, "Player: [M] Map CLOSED");
    }
}

void PlayerInput::HandleTestDeath(Player* player)
{
    if (!IsKeyPressed(KEY_K))
        return;

    if (!player->bIsAlive)
        return;

    player->bIsAlive = false;
    player->State = PLAYER_DEAD;
    
    player->currentState = DEAD;
    player->frame = 0;
    player->isDead = true;

    player->bInventoryOpen = false;
    player->bMapOpen = false;
    TraceLog(LOG_INFO, "Player: [K] TEST DEATH — Player is now DEAD!");
}

void PlayerInput::HandleTestRevive(Player* player)
{
    if (!IsKeyPressed(KEY_R))
        return;

    if (player->bIsAlive)
        return; 

    player->bIsAlive = true;
    player->State = PLAYER_IDLE;

    player->currentState = IDLE;
    player->isDead = false;

    TraceLog(LOG_INFO, "Player: [R] TEST REVIVE — Player is now ALIVE!");
}
