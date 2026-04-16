#include "../include/hud.h"
#include "../include/player.h"
#include "../include/animation.h"
#include <cstdio>

/**
 * @brief Helper untuk menggambar teks dengan shadow (bayangan) agar terbaca di background terang/gelap.
 */
static void DrawTextHUD(const char* text, int x, int y, int fontSize, Color color)
{
    DrawText(text, x + 2, y + 2, fontSize, ColorAlpha(BLACK, 0.6f)); // Shadow
    DrawText(text, x, y, fontSize, color);                       // Main Text
}

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
    DrawTextHUD(buffer, (int)(pos.x + width + 15), (int)pos.y + 1, 18, WHITE);
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

        // 4. Item Info (Icon)
        InventoryItem item = PlayerInstance.GetHotbarItem(i);
        if (item.type != ITEM_NONE)
        {
            // Ambil source rect dari test.png
            Rectangle src = GetFrame(item.iconX, item.iconY);
            
            // Render icon ditengah slot, sedikit diperbesar (misal 50x50)
            float iconDrawSize = 50.0f;
            Rectangle dest = {
                slotRect.x + (slotRect.width - iconDrawSize) / 2.0f,
                slotRect.y + (slotRect.height - iconDrawSize) / 2.0f,
                iconDrawSize,
                iconDrawSize
            };
            
            DrawTexturePro(TexturesMap[TEXTURE_ITEMS], src, dest, {0, 0}, 0.0f, WHITE);

            // Draw amount if > 1 or it's a potion/food
            if (item.amount > 0)
            {
                char amtBuf[12];
                sprintf(amtBuf, "%d", item.amount);
                int textW = MeasureText(amtBuf, 12);
                DrawTextHUD(amtBuf, (int)(slotRect.x + slotRect.width - textW - 5), (int)(slotRect.y + slotRect.height - 15), 12, WHITE);
            }
        }

        // 5. Slot Number (Key bind)
        char keyBuf[4];
        sprintf(keyBuf, "%d", i + 1);
        DrawTextHUD(keyBuf, (int)slotRect.x + 2, (int)slotRect.y - 12, 10, GRAY);
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
    const float barWidth = 220.0f;
    const float barHeight = 22.0f;
    const float padding = 30.0f;
    const float gap = 8.0f;
    const float avatarSize = 80.0f;
    const float avatarPadding = 18.0f;

    // Posisi Avatar
    Vector2 avatarPos = { padding + avatarSize/2.0f, (float)GameScreenHeight - padding - avatarSize/2.0f };
    float radius = avatarSize / 2.0f;
    
    // Background & Circle Avatar (Premium Glow & Circle)
    DrawCircleV({ avatarPos.x + 2, avatarPos.y + 2 }, radius + 2, ColorAlpha(BLACK, 0.4f)); // Shadow
    DrawCircleV(avatarPos, radius, DARKGRAY);
    
    // Knight Sprite (Down Idle) - Dibuat sesuai ukuran frame yang lebih besar
    Rectangle knightSrc = GetFrame(0, 2);
    float spriteSize = avatarSize - 10.0f; 
    Rectangle knightDest = { 
        (avatarPos.x - spriteSize/2.0f) + 1.0f, 
        avatarPos.y - spriteSize/2.0f, 
        spriteSize, 
        spriteSize 
    };
    DrawTexturePro(TexturesMap[TEXTURE_KNIGHT], knightSrc, knightDest, { 0, 0 }, 0.0f, WHITE);
    
    // Border Avatar (Circular)
    DrawCircleLinesV(avatarPos, radius, ColorAlpha(GOLD, 0.6f));
    DrawCircleLinesV(avatarPos, radius + 1, ColorAlpha(GOLD, 0.3f)); // Subtle outer glow

    // Posisi Bars (Geser ke kanan avatar)
    float barsX = padding + avatarSize + avatarPadding;
    Vector2 healthPos = { barsX, (float)GameScreenHeight - padding - (barHeight * 2) - gap };
    Vector2 manaPos = { barsX, (float)GameScreenHeight - padding - barHeight };

    // Draw Name (Di atas HP bar) dengan Shadow dan warna putih agar premium
    DrawTextHUD(PlayerInstance.GetName(), (int)healthPos.x, (int)healthPos.y - 25, 20, WHITE);

    // Draw Bars
    DrawStatBar(healthPos, barWidth, barHeight, healthRatio, healthColor, "HP", (int)health, (int)maxHealth);
    DrawStatBar(manaPos, barWidth, barHeight, manaRatio, SKYBLUE, "MP", (int)mana, (int)maxMana);

    // Draw Hotbar
    DrawHotbar();
}
