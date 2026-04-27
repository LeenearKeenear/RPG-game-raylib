#include "../include/hud.h"
#include "../include/player.h"
#include "../include/animation.h"
#include "../include/inventory.h"
#include "../include/effectQueue.h"
#include <cstdio>
#include <vector>
#include <string>

extern const int GameScreenWidth;
extern const int GameScreenHeight;

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

static int selectedSlot = -1; // -1: none, 0-48: bag, 49-52: hotbar

static InventoryItem& GetItemBySlotIndex(int index) {
    if (index >= 0 && index < 49) return PlayerInstance.Bag[index];
    if (index >= 49 && index < 53) return PlayerInstance.Hotbar[index - 49];
    static InventoryItem empty = {ITEM_NONE, "", 0, 0, 0, 0, 0};
    return empty;
}

void DrawInventory()
{
    if (!InputInstance.IsInventoryOpen()) {
        selectedSlot = -1;
        return;
    }

    // Overlay gelap
    DrawRectangle(0, 0, GameScreenWidth, GameScreenHeight, ColorAlpha(BLACK, 0.7f));
    DrawTextHUD("INVENTORY", GameScreenWidth / 2 - 90, 50, 30, GOLD);

    const float slotSize = 50.0f;
    const float padding = 6.0f;
    const int gridSize = 7;
    const float totalGridSize = (slotSize * gridSize) + (padding * (gridSize - 1));
    
    const float startX = (GameScreenWidth - totalGridSize) / 2.0f;
    const float startY = (GameScreenHeight - totalGridSize) / 2.0f;

    Vector2 mousePos = GetVirtualMousePosition(gState);
    bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    for (int i = 0; i < 49; i++)
    {
        int row = i / gridSize;
        int col = i % gridSize;

        Rectangle slotRect = {
            startX + col * (slotSize + padding),
            startY + row * (slotSize + padding),
            slotSize,
            slotSize
        };

        bool isHovered = CheckCollisionPointRec(mousePos, slotRect);
        bool isSelected = (selectedSlot == i);

        // Slot background
        Color slotColor = isSelected ? ColorAlpha(GOLD, 0.4f) : (isHovered ? ColorAlpha(GRAY, 0.7f) : ColorAlpha(DARKGRAY, 0.6f));
        DrawRectangleRounded(slotRect, 0.2f, 8, slotColor);
        DrawRectangleRoundedLines(slotRect, 0.2f, 8, isSelected ? GOLD : ColorAlpha(WHITE, 0.3f));

        // Item icon
        InventoryItem& item = PlayerInstance.Bag[i];
        if (item.type != ITEM_NONE)
        {
            float iconSize = 36.0f;
            Rectangle dest = {
                slotRect.x + (slotSize - iconSize) / 2.0f,
                slotRect.y + (slotSize - iconSize) / 2.0f,
                iconSize,
                iconSize
            };
            DrawTileTexture(TEXTURE_ITEMS, item.iconX, item.iconY, dest);

            if (item.amount > 1)
            {
                char amtBuf[12];
                sprintf(amtBuf, "%d", item.amount);
                DrawText(amtBuf, (int)slotRect.x + 35, (int)slotRect.y + 35, 12, WHITE);
            }
        }

        // Logic Click
        if (isHovered && mouseClicked)
        {
            if (selectedSlot == -1) 
            {
                if (item.type != ITEM_NONE) selectedSlot = i;
            }
            else 
            {
                // Swap
                InventoryItem temp = GetItemBySlotIndex(i);
                GetItemBySlotIndex(i) = GetItemBySlotIndex(selectedSlot);
                GetItemBySlotIndex(selectedSlot) = temp;
                selectedSlot = -1;
            }
        }
    }

    DrawTextHUD("Press 'I' to Close", GameScreenWidth / 2 - 85, GameScreenHeight - 60, 20, GRAY);
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
        bool isInventoryOpen = InputInstance.IsInventoryOpen();

        int globalIdx = 49 + i;
        bool isHovered = isInventoryOpen && CheckCollisionPointRec(GetVirtualMousePosition(gState), slotRect);
        bool isSelected = (selectedSlot == globalIdx);

        DrawRectangleRounded((Rectangle){slotRect.x + 2, slotRect.y + 2, slotRect.width, slotRect.height}, 0.2f, 8, ColorAlpha(BLACK, 0.4f));

        Color bgColor = isActive ? ColorAlpha(GOLD, 0.3f) : (isSelected ? ColorAlpha(GOLD, 0.4f) : ColorAlpha(DARKGRAY, 0.6f));
        if (isHovered && !isSelected) bgColor = ColorAlpha(GRAY, 0.7f);
        
        DrawRectangleRounded(slotRect, 0.4f, 8, bgColor);

        Color borderColor = (isActive || isSelected) ? GOLD : ColorAlpha(WHITE, 0.3f);
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

        // Logic Click in Hotbar (when inventory open)
        if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (selectedSlot == -1)
            {
                if (item.type != ITEM_NONE) selectedSlot = globalIdx;
            }
            else
            {
                // Swap
                InventoryItem temp = GetItemBySlotIndex(globalIdx);
                GetItemBySlotIndex(globalIdx) = GetItemBySlotIndex(selectedSlot);
                GetItemBySlotIndex(selectedSlot) = temp;
                selectedSlot = -1;
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
    DrawInventory();
}
