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

static const int STACK_LIMIT = 8;

// Drag state
static int dragSlot = -1;                // slot asal drag (-1 = tidak ada)
static InventoryItem dragItem = {-1, 0}; // copy item yang sedang di-drag

static void DrawItemIcon(const InventoryItem &item, Rectangle dest)
{
    if (item.definitionId == -1)
        return;
    const ItemDefinition &def = itemDefs.Get(item.definitionId);
    DrawTileTexture(TEXTURE_ITEMS, (int)def.sheetCoord.x, (int)def.sheetCoord.y, dest);
}

static void DrawTextHUD(const char *text, int x, int y, int fontSize, Color color)
{
    int textWidth = MeasureText(text, fontSize);
    float padX = (float)fontSize * 0.8f;
    float padY = (float)fontSize * 0.5f;
    DrawRectangleRounded(
        (Rectangle){(float)x - padX / 2.0f, (float)y - padY / 2.0f, (float)textWidth + padX, (float)fontSize + padY},
        0.3f, 8, ColorAlpha(BLACK, 0.8f));
    DrawText(text, x, y, fontSize, color);
}

static InventoryItem &GetItemBySlotIndex(int index)
{
    if (index >= 0 && index < 49)
        return PlayerInstance.Bag[index];
    if (index >= 49 && index < 53)
        return PlayerInstance.Hotbar[index - 49];
    static InventoryItem empty = {-1, 0};
    return empty;
}

// Handle drop ke slot tujuan — merge atau swap
static void HandleDrop(int toSlot)
{
    if (toSlot == dragSlot)
    {
        dragSlot = -1;
        dragItem = {-1, 0};
        return;
    }

    InventoryItem &src = GetItemBySlotIndex(dragSlot);
    InventoryItem &dst = GetItemBySlotIndex(toSlot);

    if (dragItem.definitionId == -1)
    {
        dragSlot = -1;
        return;
    }

    // Cek apakah bisa di-stack
    bool canStack = false;
    if (dst.definitionId == dragItem.definitionId && dst.definitionId != -1)
    {
        const ItemDefinition &def = itemDefs.Get(dragItem.definitionId);
        canStack = (def.category != ITEM_WEAPON);
    }

    if (canStack)
    {
        int spaceLeft = STACK_LIMIT - dst.amount;
        if (spaceLeft <= 0)
        {
            // Penuh — swap biasa
            InventoryItem temp = dst;
            dst = dragItem;
            src = temp;
        }
        else if (dragItem.amount <= spaceLeft)
        {
            // Semua muat — merge penuh, slot asal kosong
            dst.amount += dragItem.amount;
            src = {-1, 0};
        }
        else
        {
            // Sebagian muat — partial merge
            dst.amount += spaceLeft;
            src.amount = dragItem.amount - spaceLeft;
        }
    }
    else
    {
        // Swap biasa
        InventoryItem temp = dst;
        dst = dragItem;
        src = temp;
    }

    dragSlot = -1;
    dragItem = {-1, 0};
}

// Render ghost icon ikut cursor
static void DrawDragGhost(Vector2 mousePos)
{
    if (dragSlot == -1 || dragItem.definitionId == -1)
        return;
    float ghostSize = 36.0f;
    Rectangle dest = {
        mousePos.x - ghostSize / 2.0f,
        mousePos.y - ghostSize / 2.0f,
        ghostSize, ghostSize};
    DrawItemIcon(dragItem, dest);
}

void DrawInventory()
{
    if (!InputInstance.IsInventoryOpen())
    {
        // Cancel drag kalau inventory ditutup
        if (dragSlot != -1)
        {
            dragSlot = -1;
            dragItem = {-1, 0};
        }
        return;
    }

    DrawRectangle(0, 0, GameScreenWidth, GameScreenHeight, ColorAlpha(BLACK, 0.7f));
    DrawTextHUD("INVENTORY", GameScreenWidth / 2 - 90, 50, 30, GOLD);

    const float slotSize = 50.0f;
    const float padding = 6.0f;
    const int gridSize = 7;
    const float totalGridSize = (slotSize * gridSize) + (padding * (gridSize - 1));
    const float startX = (GameScreenWidth - totalGridSize) / 2.0f;
    const float startY = (GameScreenHeight - totalGridSize) / 2.0f;

    Vector2 mousePos = GetVirtualMousePosition(gState);
    bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

    for (int i = 0; i < 49; i++)
    {
        int row = i / gridSize;
        int col = i % gridSize;

        Rectangle slotRect = {
            startX + col * (slotSize + padding),
            startY + row * (slotSize + padding),
            slotSize, slotSize};

        bool isHovered = CheckCollisionPointRec(mousePos, slotRect);
        bool isDragSource = (dragSlot == i);

        Color slotColor = isDragSource ? ColorAlpha(GOLD, 0.2f) : (isHovered ? ColorAlpha(GRAY, 0.7f) : ColorAlpha(DARKGRAY, 0.6f));
        DrawRectangleRounded(slotRect, 0.2f, 8, slotColor);
        DrawRectangleRoundedLines(slotRect, 0.2f, 8, isDragSource ? ColorAlpha(GOLD, 0.5f) : ColorAlpha(WHITE, 0.3f));

        InventoryItem &item = PlayerInstance.Bag[i];

        // Kalau ini slot asal drag, render dengan transparansi
        if (item.definitionId != -1 && !isDragSource)
        {
            float iconSize = 36.0f;
            Rectangle dest = {
                slotRect.x + (slotSize - iconSize) / 2.0f,
                slotRect.y + (slotSize - iconSize) / 2.0f,
                iconSize, iconSize};
            DrawItemIcon(item, dest);

            if (item.amount > 1)
            {
                char amtBuf[12];
                sprintf(amtBuf, "%d", item.amount);
                DrawText(amtBuf, (int)slotRect.x + 35, (int)slotRect.y + 35, 12, WHITE);
            }
        }

        // Mulai drag
        if (isHovered && mousePressed && dragSlot == -1 && item.definitionId != -1)
        {
            dragSlot = i;
            dragItem = item;
        }

        // Drop ke slot ini
        if (isHovered && mouseReleased && dragSlot != -1 && dragSlot != i)
            HandleDrop(i);
    }

    // Drop di luar slot — cancel drag, item balik
    if (mouseReleased && dragSlot != -1)
    {
        dragSlot = -1;
        dragItem = {-1, 0};
    }

    DrawTextHUD("Press 'I' to Close", GameScreenWidth / 2 - 85, GameScreenHeight - 60, 20, GRAY);
}

static void DrawStatBar(Vector2 pos, float width, float height, float ratio, Color color, int current)
{
    DrawRectangleRounded((Rectangle){pos.x + 2, pos.y + 2, width, height}, 0.4f, 8, ColorAlpha(BLACK, 0.3f));
    DrawRectangleRounded((Rectangle){pos.x, pos.y, width, height}, 0.4f, 8, DARKGRAY);

    if (ratio > 0)
    {
        DrawRectangleRounded((Rectangle){pos.x, pos.y, width * ratio, height}, 0.4f, 8, color);
        DrawRectangleRounded((Rectangle){pos.x, pos.y, width * ratio, height * 0.4f}, 0.4f, 8, ColorAlpha(WHITE, 0.1f));
    }

    DrawRectangleRoundedLines((Rectangle){pos.x, pos.y, width, height}, 0.4f, 8, ColorAlpha(WHITE, 0.2f));

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
    bool isInventoryOpen = InputInstance.IsInventoryOpen();

    Vector2 mousePos = GetVirtualMousePosition(gState);
    bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

    for (int i = 0; i < 4; i++)
    {
        Rectangle slotRect = {startX + (i * (slotSize + padding)), startY, slotSize, slotSize};
        bool isActive = (activeSlot == (int)(i + 1));
        int globalIdx = 49 + i;
        bool isHovered = isInventoryOpen && CheckCollisionPointRec(mousePos, slotRect);
        bool isDragSource = (dragSlot == globalIdx);

        DrawRectangleRounded((Rectangle){slotRect.x + 2, slotRect.y + 2, slotRect.width, slotRect.height}, 0.2f, 8, ColorAlpha(BLACK, 0.4f));

        Color bgColor = isActive ? ColorAlpha(GOLD, 0.3f) : (isDragSource ? ColorAlpha(GOLD, 0.2f) : ColorAlpha(DARKGRAY, 0.6f));
        if (isHovered && !isDragSource)
            bgColor = ColorAlpha(GRAY, 0.7f);
        DrawRectangleRounded(slotRect, 0.4f, 8, bgColor);

        Color borderColor = (isActive || isDragSource) ? GOLD : ColorAlpha(WHITE, 0.3f);
        DrawRectangleRoundedLines(slotRect, 0.4f, 8, borderColor);

        InventoryItem item = PlayerInstance.Hotbar[i];
        if (item.definitionId != -1 && !isDragSource)
        {
            float iconDrawSize = 42.0f;
            Rectangle dest = {
                slotRect.x + (slotRect.width - iconDrawSize) / 2.0f,
                slotRect.y + (slotRect.height - iconDrawSize) / 2.0f,
                iconDrawSize, iconDrawSize};
            DrawItemIcon(item, dest);

            if (item.amount > 0)
            {
                char amtBuf[12];
                sprintf(amtBuf, "%d", item.amount);
                int fontSize = 12;
                int textW = MeasureText(amtBuf, fontSize);
                DrawTextHUD(amtBuf, (int)(slotRect.x + slotRect.width - textW - 4), (int)(slotRect.y + slotRect.height - 13.5), fontSize, WHITE);
            }
        }

        if (isInventoryOpen)
        {
            // Mulai drag dari hotbar
            if (isHovered && mousePressed && dragSlot == -1 && item.definitionId != -1)
            {
                dragSlot = globalIdx;
                dragItem = item;
            }

            // Drop ke hotbar slot ini
            if (isHovered && mouseReleased && dragSlot != -1 && dragSlot != globalIdx)
                HandleDrop(globalIdx);
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

    const float barWidth = 220.0f;
    const float barHeight = 22.0f;
    const float padding = 30.0f;
    const float gap = 8.0f;
    const float avatarSize = 80.0f;
    const float avatarPadding = 18.0f;

    Vector2 avatarPos = {padding + avatarSize / 2.0f, (float)GameScreenHeight - padding - avatarSize / 2.0f};
    float radius = avatarSize / 2.0f;

    DrawCircleV({avatarPos.x + 2, avatarPos.y + 2}, radius + 2, ColorAlpha(BLACK, 0.4f));
    DrawCircleV(avatarPos, radius, DARKGRAY);

    float spriteSize = avatarSize - 10.0f;
    Rectangle knightDest = {
        (avatarPos.x - spriteSize / 2.0f) + 1.0f,
        avatarPos.y - spriteSize / 2.0f,
        spriteSize, spriteSize};
    DrawTileTexture(TEXTURE_KNIGHT, 0, 2, knightDest);

    DrawCircleLinesV(avatarPos, radius, ColorAlpha(GOLD, 0.6f));
    DrawCircleLinesV(avatarPos, radius + 1, ColorAlpha(GOLD, 0.3f));

    float barsX = padding + avatarSize + avatarPadding;
    Vector2 healthPos = {barsX, (float)GameScreenHeight - padding - (barHeight * 2) - gap};
    Vector2 manaPos = {barsX, (float)GameScreenHeight - padding - barHeight};

    DrawTextHUD(PlayerInstance.GetName(), (int)healthPos.x + 7, (int)healthPos.y - 35, 20, WHITE);
    DrawStatBar(healthPos, barWidth, barHeight, healthRatio, RED, (int)health);
    DrawStatBar(manaPos, barWidth, barHeight, manaRatio, GOLD, (int)mana);

    DrawHotbar();
    DrawInventory();
    if (InputInstance.IsInventoryOpen())
        DrawDragGhost(GetVirtualMousePosition(gState));
}