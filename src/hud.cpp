#include "../include/hud.h"
#include "../include/player.h"
#include <cstdio>

/**
 * @brief Helper untuk menggambar bar statistik (HP/MP) dengan teks di kanannya.
 * Dibuat internal di file ini saja.
 */
static void DrawStatBar(Vector2 pos, float width, float height, float ratio, Color color, const char* label, int current, int max)
{
    // 1. Shadow & Background
    DrawRectangleRounded((Rectangle){ pos.x + 2, pos.y + 2, width, height }, 0.4f, 8, ColorAlpha(BLACK, 0.3f));
    DrawRectangleRounded((Rectangle){ pos.x, pos.y, width, height }, 0.4f, 8, DARKGRAY);
    
    // 2. Fill (dengan efek gloss)
    if (ratio > 0) {
        DrawRectangleRounded((Rectangle){ pos.x, pos.y, width * ratio, height }, 0.4f, 8, color);
        DrawRectangleRounded((Rectangle){ pos.x, pos.y, width * ratio, height * 0.4f }, 0.4f, 8, ColorAlpha(WHITE, 0.1f));
    }

    // 3. Border
    DrawRectangleRoundedLines((Rectangle){ pos.x, pos.y, width, height }, 0.4f, 8, ColorAlpha(WHITE, 0.2f));

    // 4. Text (Kanan Bar)
    char buffer[32];
    sprintf(buffer, "%s: %d/%d", label, current, max);
    DrawText(buffer, (int)(pos.x + width + 15), (int)pos.y + 1, 18, WHITE);
}

void DrawHotbar()
{
    extern const int GameScreenWidth;
    extern const int GameScreenHeight;

    const float slotSize = 65.0f; // Kotak diperbesar
    const float padding = 12.0f;
    const float screenPadding = 30.0f; // Menyamakan dengan padding HP bar
    const float totalWidth = (slotSize * 4) + (padding * 3);
    
    // Taruh di kanan bawah
    const float startX = (float)GameScreenWidth - screenPadding - totalWidth;
    const float startY = (float)GameScreenHeight - screenPadding - slotSize;

    ItemSlot activeSlot = InputInstance.GetActiveSlot();

    for (int i = 0; i < 4; i++)
    {
        Rectangle slotRect = {startX + (i * (slotSize + padding)), startY, slotSize, slotSize};
        bool isActive = (activeSlot == (int)(i + 1));

        // 1. Shadow
        DrawRectangleRounded((Rectangle){slotRect.x + 2, slotRect.y + 2, slotRect.width, slotRect.height}, 0.2f, 8, ColorAlpha(BLACK, 0.4f));

        // 2. Background
        Color bgColor = isActive ? ColorAlpha(GOLD, 0.3f) : ColorAlpha(DARKGRAY, 0.6f);
        DrawRectangleRounded(slotRect, 0.2f, 8, bgColor);

        // 3. Border
        Color borderColor = isActive ? GOLD : ColorAlpha(WHITE, 0.3f);
        DrawRectangleRoundedLines(slotRect, 0.2f, 8, borderColor);

        // 4. Item Info
        InventoryItem item = PlayerInstance.GetHotbarItem(i);
        if (item.type != ITEM_NONE)
        {
            // Draw item name abbreviation or icon-like text
            const char* label = (item.type == ITEM_WEAPON) ? "W" : "P";
            Color labelColor = (item.type == ITEM_WEAPON) ? LIGHTGRAY : SKYBLUE;
            
            DrawText(label, (int)slotRect.x + 8, (int)slotRect.y + 8, 15, labelColor);

            // Draw amount if > 1 or it's a potion
            if (item.amount > 0)
            {
                char amtBuf[12];
                sprintf(amtBuf, "%d", item.amount);
                int textW = MeasureText(amtBuf, 12);
                DrawText(amtBuf, (int)(slotRect.x + slotRect.width - textW - 5), (int)(slotRect.y + slotRect.height - 15), 12, WHITE);
            }
        }

        // 5. Slot Number (Key bind)
        char keyBuf[4];
        sprintf(keyBuf, "%d", i + 1);
        DrawText(keyBuf, (int)slotRect.x + 2, (int)slotRect.y - 12, 10, GRAY);
    }
}

void DrawPlayerHUD()
{
    // Data stats
    float health = PlayerInstance.GetHealth();
    float maxHealth = PlayerInstance.GetMaxHealth();
    float healthRatio = (maxHealth > 0) ? health / maxHealth : 0;
    
    float mana = PlayerInstance.GetMana();
    float maxMana = PlayerInstance.GetMaxMana();
    float manaRatio = (maxMana > 0) ? mana / maxMana : 0;

    // Warna HP dinamis
    Color healthColor = GREEN;
    if (healthRatio < 0.25f) healthColor = RED;
    else if (healthRatio < 0.5f) healthColor = ORANGE;

    // Konfigurasi layout
    extern const int GameScreenHeight; 
    const float barWidth = 200.0f;
    const float barHeight = 22.0f;
    const float padding = 30.0f;
    const float gap = 12.0f;

    Vector2 healthPos = { padding, (float)GameScreenHeight - padding - (barHeight * 2) - gap };
    Vector2 manaPos = { padding, (float)GameScreenHeight - padding - barHeight };

    // Draw Name
    DrawText(PlayerInstance.GetName(), (int)healthPos.x, (int)healthPos.y - 25, 20, GOLD);

    // Draw Bars
    DrawStatBar(healthPos, barWidth, barHeight, healthRatio, healthColor, "HP", (int)health, (int)maxHealth);
    DrawStatBar(manaPos, barWidth, barHeight, manaRatio, SKYBLUE, "MP", (int)mana, (int)maxMana);

    // Draw Hotbar
    DrawHotbar();
}
