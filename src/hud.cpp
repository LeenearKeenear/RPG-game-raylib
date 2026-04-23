#include "../include/hud.h"
#include "../include/player.h"
#include "../include/tiles.h"
#include "../include/animation.h"
#include <cstdio>

/**
 * @brief Helper untuk menggambar teks dengan shadow (bayangan) agar terbaca di background terang/gelap.
 */
static void DrawTextHUD(const char* text, int x, int y, int fontSize, Color color)
{
    // 1. Hitung lebar teks
    int textWidth = MeasureText(text, fontSize);
    
    // 2. Padding background proporsional terhadap ukuran font
    float padX = (float)fontSize * 0.8f;
    float padY = (float)fontSize * 0.5f;
    
    // 3. Gambar background hitam semi-transparan
    DrawRectangleRounded(
        (Rectangle){ (float)x - padX/2.0f, (float)y - padY/2.0f, (float)textWidth + padX, (float)fontSize + padY }, 
        0.3f, 8, ColorAlpha(BLACK, 0.8f)
    );

    // 4. Gambar teks utama
    DrawText(text, x, y, fontSize, color);
}

/**
 * @brief Helper untuk menggambar bar statistik (HP/MP) dengan teks di kanannya.
 * Dibuat internal di file ini saja.
 */
static void DrawStatBar(Vector2 pos, float width, float height, float ratio, Color color, int current)
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

    // 4. Text (Samping Bar)
    char buffer[32];
    sprintf(buffer, "%d", current);
    
    int fontSize = 18; // Ukuran disesuaikan dengan tinggi bar (22)
    
    // Posisikan di kanan bar
    int textX = (int)(pos.x + width + 15);
    int textY = (int)(pos.y + (height - fontSize) / 2.0f); // Rata tengah vertikal
    
    DrawTextHUD(buffer, textX, textY, fontSize, WHITE);
}

void DrawHotbar()
{
    extern const int GameScreenWidth;
    extern const int GameScreenHeight;

    const float slotSize = 55.0f; // Ukuran kotak dikurangi
    const float padding = 10.0f;
    const float screenPadding = 30.0f; // Menyamakan dengan padding HP bar
    const float totalWidth = (slotSize * 4) + (padding * 3);
    
    // Taruh di kanan bawah, sejajar dengan bagian bawah mana bar (GameScreenHeight - 30)
    const float startX = (float)GameScreenWidth - screenPadding - totalWidth;
    const float startY = (float)GameScreenHeight - 30.0f - slotSize;

    ItemSlot activeSlot = InputInstance.GetActiveSlot();

    for (int i = 0; i < 4; i++)
    {
        Rectangle slotRect = {startX + (i * (slotSize + padding)), startY, slotSize, slotSize};
        bool isActive = (activeSlot == (int)(i + 1));

        // 1. Shadow
        DrawRectangleRounded((Rectangle){slotRect.x + 2, slotRect.y + 2, slotRect.width, slotRect.height}, 0.2f, 8, ColorAlpha(BLACK, 0.4f));

        // 2. Background
        Color bgColor = isActive ? ColorAlpha(GOLD, 0.3f) : ColorAlpha(DARKGRAY, 0.6f);
        DrawRectangleRounded(slotRect, 0.4f, 8, bgColor);

        // 3. Border
        Color borderColor = isActive ? GOLD : ColorAlpha(WHITE, 0.3f);
        DrawRectangleRoundedLines(slotRect, 0.4f, 8, borderColor);

        // 4. Item Info (Icon)
        InventoryItem item = PlayerInstance.GetHotbarItem(i);
        if (item.type != ITEM_NONE)
        {
            // Ambil source rect dari test.png
            Rectangle src = GetFrame(item.iconX, item.iconY);
            
            // Render icon ditengah slot, sedikit diperbesar (misal 42x42)
            float iconDrawSize = 42.0f;
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
                int fontSize = 12; // Tetap besar sesuai permintaan
                int textW = MeasureText(amtBuf, fontSize);
                // Sesuaikan posisi Y untuk slotSize 55 (menggunakan offset -22 agar terlihat pas di pojok)
                DrawTextHUD(amtBuf, (int)(slotRect.x + slotRect.width - textW - 4), (int)(slotRect.y + slotRect.height - 13.5), fontSize, WHITE);
            }
        }
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
    DrawTextHUD(PlayerInstance.GetName(), (int)healthPos.x + 7, (int)healthPos.y - 35, 20, WHITE);

    // Draw Bars
    DrawStatBar(healthPos, barWidth, barHeight, healthRatio, healthColor, (int)health);
    DrawStatBar(manaPos, barWidth, barHeight, manaRatio, SKYBLUE, (int)mana);

    // Draw Hotbar
    DrawHotbar();
}
