#include "../include/screen.h"
#include "../include/player.h"
#include "../include/tiles.h"
#include "../include/animation.h"
#include <cstdio>

/**
 * Me-render teks dengan latar belakang bulat semi-transparan untuk keterbacaan yang lebih baik.
 */
static void DrawTextHUD(const char* text, int x, int y, int fontSize, Color color)
{
    int textWidth = MeasureText(text, fontSize);
    
    float padX = (float)fontSize * 0.8f;
    float padY = (float)fontSize * 0.5f;
    
    DrawRectangleRounded(
        (Rectangle){ (float)x - padX/2.0f, (float)y - padY/2.0f, (float)textWidth + padX, (float)fontSize + padY }, 
        0.3f, 8, ColorAlpha(BLACK, 0.8f)
    );

    DrawText(text, x, y, fontSize, color);
}

/**
 * Menggambar bar progres untuk kesehatan (HP) atau mana.
 * Mencakup bayangan, batas luar, dan label nilai numerik.
 */
static void DrawStatBar(Vector2 pos, float width, float height, float ratio, Color color, int current)
{
    // Latar belakang dan bayangan
    DrawRectangleRounded((Rectangle){ pos.x + 2, pos.y + 2, width, height }, 0.4f, 8, ColorAlpha(BLACK, 0.3f));
    DrawRectangleRounded((Rectangle){ pos.x, pos.y, width, height }, 0.4f, 8, DARKGRAY);
    
    // Bagian yang terisi
    if (ratio > 0) {
        DrawRectangleRounded((Rectangle){ pos.x, pos.y, width * ratio, height }, 0.4f, 8, color);
        // Kilatan efek glossy
        DrawRectangleRounded((Rectangle){ pos.x, pos.y, width * ratio, height * 0.4f }, 0.4f, 8, ColorAlpha(WHITE, 0.1f));
    }

    // Batas luar (Border)
    DrawRectangleRoundedLines((Rectangle){ pos.x, pos.y, width, height }, 0.4f, 8, ColorAlpha(WHITE, 0.2f));

    // Teks nilai numerik
    char buffer[32];
    sprintf(buffer, "%d", current);
    int fontSize = 18;
    int textX = (int)(pos.x + width + 15);
    int textY = (int)(pos.y + (height - fontSize) / 2.0f);
    DrawTextHUD(buffer, textX, textY, fontSize, WHITE);
}

/**
 * Me-render slot item akses cepat pemain (Hotbar).
 * Menyoroti slot yang aktif dan menampilkan ikon item serta jumlahnya.
 */
void DrawHotbar()
{
    extern const int GameScreenWidth;
    extern const int GameScreenHeight;

    const float slotSize = 55.0f;
    const float padding = 10.0f;
    const float screenPadding = 30.0f;
    const float totalWidth = (slotSize * 4) + (padding * 3);
    
    // Posisikan hotbar di pojok kanan bawah layar
    const float startX = (float)GameScreenWidth - screenPadding - totalWidth;
    const float startY = (float)GameScreenHeight - 30.0f - slotSize;

    ItemSlot activeSlot = InputInstance.GetActiveSlot();

    for (int i = 0; i < 4; i++)
    {
        Rectangle slotRect = {startX + (i * (slotSize + padding)), startY, slotSize, slotSize};
        bool isActive = (activeSlot == (int)(i + 1));

        // Latar belakang slot
        DrawRectangleRounded((Rectangle){slotRect.x + 2, slotRect.y + 2, slotRect.width, slotRect.height}, 0.2f, 8, ColorAlpha(BLACK, 0.4f));
        Color bgColor = isActive ? ColorAlpha(GOLD, 0.3f) : ColorAlpha(DARKGRAY, 0.6f);
        DrawRectangleRounded(slotRect, 0.4f, 8, bgColor);

        // Batas luar slot aktif
        Color borderColor = isActive ? GOLD : ColorAlpha(WHITE, 0.3f);
        DrawRectangleRoundedLines(slotRect, 0.4f, 8, borderColor);

        // Ikon item dan jumlahnya
        InventoryItem item = PlayerInstance.GetHotbarItem(i);
        if (item.type != ITEM_NONE)
        {
            Rectangle src = GetFrame(item.iconX, item.iconY);
            float iconDrawSize = 42.0f;
            Rectangle dest = {
                slotRect.x + (slotRect.width - iconDrawSize) / 2.0f,
                slotRect.y + (slotRect.height - iconDrawSize) / 2.0f,
                iconDrawSize,
                iconDrawSize
            };
            DrawTexturePro(TexturesMap[TEXTURE_ITEMS], src, dest, {0, 0}, 0.0f, WHITE);

            if (item.amount > 0)
            {
                char amtBuf[12];
                sprintf(amtBuf, "%d", item.amount);
                int fontSize = 12;
                int textW = MeasureText(amtBuf, fontSize);
                DrawTextHUD(amtBuf, (int)(slotRect.x + slotRect.width - textW - 4), (int)(slotRect.y + slotRect.height - 13.5), fontSize, WHITE);
            }
        }
    }
}

/**
 * Fungsi utama render HUD.
 * Menggambar avatar, bar kesehatan, bar mana, dan nama pemain.
 */
void DrawPlayerHUD()
{
    float health = PlayerInstance.GetHealth();
    float maxHealth = PlayerInstance.GetMaxHealth();
    float healthRatio = (maxHealth > 0) ? health / maxHealth : 0;
    
    float mana = PlayerInstance.GetMana();
    float maxMana = PlayerInstance.GetMaxMana();
    float manaRatio = (maxMana > 0) ? mana / maxMana : 0;

    // Warna dinamis berdasarkan tingkat kesehatan
    Color healthColor = GREEN;
    if (healthRatio < 0.25f) healthColor = RED;
    else if (healthRatio < 0.5f) healthColor = ORANGE;

    extern const int GameScreenHeight; 
    const float barWidth = 220.0f;
    const float barHeight = 22.0f;
    const float padding = 30.0f;
    const float gap = 8.0f;
    const float avatarSize = 80.0f;
    const float avatarPadding = 18.0f;

    // --- Render Avatar ---
    Vector2 avatarPos = { padding + avatarSize/2.0f, (float)GameScreenHeight - padding - avatarSize/2.0f };
    float radius = avatarSize / 2.0f;
    
    DrawCircleV({ avatarPos.x + 2, avatarPos.y + 2 }, radius + 2, ColorAlpha(BLACK, 0.4f));
    DrawCircleV(avatarPos, radius, DARKGRAY);
    
    // Gambar potret karakter (frame 0, baris 2)
    Rectangle knightSrc = GetFrame(0, 2);
    float spriteSize = avatarSize - 10.0f; 
    Rectangle knightDest = { 
        (avatarPos.x - spriteSize/2.0f) + 1.0f, 
        avatarPos.y - spriteSize/2.0f, 
        spriteSize, 
        spriteSize 
    };
    DrawTexturePro(TexturesMap[TEXTURE_KNIGHT], knightSrc, knightDest, { 0, 0 }, 0.0f, WHITE);
    
    // Cincin dekoratif di sekitar avatar
    DrawCircleLinesV(avatarPos, radius, ColorAlpha(GOLD, 0.6f));
    DrawCircleLinesV(avatarPos, radius + 1, ColorAlpha(GOLD, 0.3f));

    // --- Bar dan Teks ---
    float barsX = padding + avatarSize + avatarPadding;
    Vector2 healthPos = { barsX, (float)GameScreenHeight - padding - (barHeight * 2) - gap };
    Vector2 manaPos = { barsX, (float)GameScreenHeight - padding - barHeight };

    DrawTextHUD(PlayerInstance.GetName(), (int)healthPos.x + 7, (int)healthPos.y - 35, 20, WHITE);

    DrawStatBar(healthPos, barWidth, barHeight, healthRatio, healthColor, (int)health);
    DrawStatBar(manaPos, barWidth, barHeight, manaRatio, SKYBLUE, (int)mana);

    DrawHotbar();
}
