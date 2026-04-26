#include "../include/hud.h"
#include "../include/player.h"
#include "../include/animation.h"
#include <cstdio>

/**
 * @brief Helper untuk menggambar teks dengan shadow (bayangan) agar terbaca di background terang/gelap.
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
 * @brief Helper untuk menggambar bar statistik (HP/MP) dengan teks di kanannya.
 */
static void DrawStatBar(Vector2 pos, float width, float height, float ratio, Color color, int current)
{
    DrawRectangleRounded((Rectangle){ pos.x + 2, pos.y + 2, width, height }, 0.4f, 8, ColorAlpha(BLACK, 0.3f));
    DrawRectangleRounded((Rectangle){ pos.x, pos.y, width, height }, 0.4f, 8, DARKGRAY);
    
    if (ratio > 0) {
        DrawRectangleRounded((Rectangle){ pos.x, pos.y, width * ratio, height }, 0.4f, 8, color);
        DrawRectangleRounded((Rectangle){ pos.x, pos.y, width * ratio, height * 0.4f }, 0.4f, 8, ColorAlpha(WHITE, 0.1f));
    }

    DrawRectangleRoundedLines((Rectangle){ pos.x, pos.y, width, height }, 0.4f, 8, ColorAlpha(WHITE, 0.2f));

    char buffer[32];
    sprintf(buffer, "%d", current);
    
    int fontSize = 18;
    int textX = (int)(pos.x + width + 15);
    int textY = (int)(pos.y + (height - fontSize) / 2.0f);
    
    DrawTextHUD(buffer, textX, textY, fontSize, WHITE);
}

void DrawHotbar()
{
    extern const int GameScreenWidth;
    extern const int GameScreenHeight;

    const float slotSize = 55.0f;
    const float padding = 10.0f;
    const float screenPadding = 30.0f;
    const float totalWidth = (slotSize * 4) + (padding * 3);
    
    const float startX = (float)GameScreenWidth - screenPadding - totalWidth;
    const float startY = (float)GameScreenHeight - 30.0f - slotSize;

    int activeSlot = (int)InputInstance.GetActiveSlot();

    for (int i = 0; i < 4; i++)
    {
        Rectangle slotRect = {startX + (i * (slotSize + padding)), startY, slotSize, slotSize};
        bool isActive = (activeSlot == (int)(i + 1));

        DrawRectangleRounded((Rectangle){slotRect.x + 2, slotRect.y + 2, slotRect.width, slotRect.height}, 0.2f, 8, ColorAlpha(BLACK, 0.4f));

        Color bgColor = isActive ? ColorAlpha(GOLD, 0.3f) : ColorAlpha(DARKGRAY, 0.6f);
        DrawRectangleRounded(slotRect, 0.4f, 8, bgColor);

        Color borderColor = isActive ? GOLD : ColorAlpha(WHITE, 0.3f);
        DrawRectangleRoundedLines(slotRect, 0.4f, 8, borderColor);

        InventoryItem item = PlayerInstance.Hotbar[i];
        if (item.type != ITEM_NONE)
        {
            float iconDrawSize = 42.0f;
            Rectangle dest = {
                slotRect.x + (slotRect.width - iconDrawSize) / 2.0f,
                slotRect.y + (slotRect.height - iconDrawSize) / 2.0f,
                iconDrawSize,
                iconDrawSize
            };
            
            DrawTileTexture(TEXTURE_ITEMS, item.iconX, item.iconY, dest);

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

void DrawPlayerHUD()
{
    float health = PlayerInstance.GetHealth();
    float maxHealth = PlayerInstance.GetMaxHealth();
    float healthRatio = (maxHealth > 0) ? health / maxHealth : 0;
    
    float mana = PlayerInstance.GetMana();
    float maxMana = PlayerInstance.GetMaxMana();
    float manaRatio = (maxMana > 0) ? mana / maxMana : 0;

    Color healthColor = RED;

    extern const int GameScreenHeight; 
    const float barWidth = 220.0f;
    const float barHeight = 22.0f;
    const float padding = 30.0f;
    const float gap = 8.0f;
    const float avatarSize = 80.0f;
    const float avatarPadding = 18.0f;

    Vector2 avatarPos = { padding + avatarSize/2.0f, (float)GameScreenHeight - padding - avatarSize/2.0f };
    float radius = avatarSize / 2.0f;
    
    DrawCircleV({ avatarPos.x + 2, avatarPos.y + 2 }, radius + 2, ColorAlpha(BLACK, 0.4f)); 
    DrawCircleV(avatarPos, radius, DARKGRAY);
    
    float spriteSize = avatarSize - 10.0f; 
    Rectangle knightDest = { 
        (avatarPos.x - spriteSize/2.0f) + 1.0f, 
        avatarPos.y - spriteSize/2.0f, 
        spriteSize, 
        spriteSize 
    };
    DrawTileTexture(TEXTURE_KNIGHT, 0, 2, knightDest);
    
    DrawCircleLinesV(avatarPos, radius, ColorAlpha(GOLD, 0.6f));
    DrawCircleLinesV(avatarPos, radius + 1, ColorAlpha(GOLD, 0.3f)); 

    float barsX = padding + avatarSize + avatarPadding;
    Vector2 healthPos = { barsX, (float)GameScreenHeight - padding - (barHeight * 2) - gap };
    Vector2 manaPos = { barsX, (float)GameScreenHeight - padding - barHeight };

    DrawTextHUD(PlayerInstance.GetName(), (int)healthPos.x + 7, (int)healthPos.y - 35, 20, WHITE);

    DrawStatBar(healthPos, barWidth, barHeight, healthRatio, healthColor, (int)health);
    DrawStatBar(manaPos, barWidth, barHeight, manaRatio, GOLD, (int)mana);

    DrawHotbar();
}
