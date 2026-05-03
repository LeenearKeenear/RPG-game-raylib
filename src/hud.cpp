#include "../include/hud.h"
#include "../include/player.h"
#include "../include/animation.h"
#include "../include/inventory.h"
#include "../include/effectQueue.h"
#include "../lib/raylib/include/raymath.h"
#include <cstdio>
#include <vector>
#include <string>

extern const int GameScreenWidth;
extern const int GameScreenHeight;

// Drag state
static int dragSlot = -1; // slot asal drag (-1 = tidak ada)
static bool isDragSplit = false;
static int splitTotalAmount = 0;
static InventoryItem dragItem = {-1, 0}; // copy item yang sedang di-drag
static std::vector<int> splitVisitedSlots;

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
    if (index >= 0 && index < PlayerInstance.MaxBag)
        return PlayerInstance.Bag[index];
    if (index >= PlayerInstance.MaxBag && index < PlayerInstance.MaxInventory)
        return PlayerInstance.Hotbar[index - PlayerInstance.MaxBag];
    static InventoryItem empty = {-1, 0};
    return empty;
}

static void HandleSplitDragSlot(int slotIndex, Rectangle slotRect, Vector2 mousePos)
{
    InventoryItem &item = GetItemBySlotIndex(slotIndex);
    bool isHovered = CheckCollisionPointRec(mousePos, slotRect);

    // Mulai drag split
    if (isHovered && InputInstance.IsRightClickDown() && dragSlot == -1 && item.definitionId != -1)
    {
        const ItemDefinition &def = itemDefs.Get(item.definitionId);
        if (def.isStackable && item.amount > 1)
        {
            splitTotalAmount = item.amount;
            dragItem = {item.definitionId, 0};
            dragSlot = slotIndex;
            isDragSplit = true;
            splitVisitedSlots.clear();
            splitVisitedSlots.push_back(slotIndex);
        }
    }

    // Hover isi slot kosong
    if (isDragSplit && isHovered)
    {
        bool alreadyVisited = std::find(splitVisitedSlots.begin(), splitVisitedSlots.end(), slotIndex) != splitVisitedSlots.end();

        if (!alreadyVisited && item.definitionId == -1)
        {
            // Stop kalau sudah tidak bisa dibagi lagi
            int nextSlotCount = (int)splitVisitedSlots.size() + 1;
            if (splitTotalAmount / nextSlotCount == 0)
                return;

            splitVisitedSlots.push_back(slotIndex);

            int slotCount = (int)splitVisitedSlots.size();
            int base = splitTotalAmount / slotCount;
            int remainder = splitTotalAmount % slotCount;

            for (int j = 0; j < slotCount; j++)
            {
                InventoryItem &s = GetItemBySlotIndex(splitVisitedSlots[j]);
                int give = base + (j == 0 ? remainder : 0);
                s = {dragItem.definitionId, give};
            }
        }
    }
}

static void HandleSplitRelease()
{
    if (InputInstance.IsRightClickReleased() && isDragSplit)
    {
        isDragSplit = false;
        dragSlot = -1;
        dragItem = {-1, 0};
        splitTotalAmount = 0;
        splitVisitedSlots.clear();
    }
}

static void HandleMergeStack(int slotIndex)
{
    InventoryItem &target = GetItemBySlotIndex(slotIndex);
    if (target.definitionId == -1)
        return;

    const ItemDefinition &def = itemDefs.Get(target.definitionId);
    if (!def.isStackable)
        return;
    if (target.amount >= def.maxStack)
        return;

    for (int i = 0; i < PlayerInstance.MaxBag && target.amount < def.maxStack; i++)
    {
        if (i == slotIndex)
            continue;
        InventoryItem &other = PlayerInstance.Bag[i];
        if (other.definitionId != target.definitionId)
            continue;
        if (other.amount >= def.maxStack)
            continue;

        int space = def.maxStack - target.amount;
        int take = std::min(space, other.amount);
        target.amount += take;
        other.amount -= take;
        if (other.amount <= 0)
            other = {-1, 0};
    }

    for (int i = 0; i < PlayerInstance.MaxHotbar && target.amount < def.maxStack; i++)
    {
        int globalIdx = PlayerInstance.MaxBag + i;
        if (globalIdx == slotIndex)
            continue;
        InventoryItem &other = PlayerInstance.Hotbar[i];
        if (other.definitionId != target.definitionId)
            continue;
        if (other.amount >= def.maxStack)
            continue;

        int space = def.maxStack - target.amount;
        int take = std::min(space, other.amount);
        target.amount += take;
        other.amount -= take;
        if (other.amount <= 0)
            other = {-1, 0};
    }
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
    const ItemDefinition &def = itemDefs.Get(dragItem.definitionId);

    if (dragItem.definitionId == -1)
    {
        dragSlot = -1;
        return;
    }

    // Cek apakah bisa di-stack
    bool canStack = false;
    if (dst.definitionId == dragItem.definitionId && dst.definitionId != -1)
    {

        canStack = (def.isStackable);
    }

    if (canStack)
    {
        int spaceLeft = def.maxStack - dst.amount;
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
        if (dragSlot != -1)
        {
            dragSlot = -1;
            dragItem = {-1, 0};
            isDragSplit = false;
            splitVisitedSlots.clear();
        }
        return;
    }

    DrawRectangle(0, 0, GameScreenWidth, GameScreenHeight, ColorAlpha(BLACK, 0.7f));
    DrawTextHUD("INVENTORY", GameScreenWidth / 2 - 90, 50, 30, GOLD);

    const float slotSize = 50.0f;
    const float padding = 6.0f;
    const int gridSize = 5;
    const float totalGridSize = (slotSize * gridSize) + (padding * (gridSize - 1));
    const float startX = (GameScreenWidth - totalGridSize) / 2.0f;
    const float startY = (GameScreenHeight - totalGridSize) / 2.0f;

    Vector2 mousePos = GetVirtualMousePosition(gState);
    bool mousePressed = InputInstance.IsLeftClickPressed();
    bool mouseReleased = InputInstance.IsLeftClickReleased();
    bool dragHandled = false;

    for (int i = 0; i < PlayerInstance.MaxBag; i++)
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

        if (isHovered && mousePressed && dragSlot == -1 && !isDragSplit && item.definitionId != -1)
        {
            bool ctrlHeld = InputInstance.IsCtrlDown();
            if (ctrlHeld)
                HandleMergeStack(i); // atau globalIdx untuk hotbar
            else
            {
                dragSlot = i;
                dragItem = item;
            };
        }

        if (isHovered && mouseReleased && dragSlot != -1 && dragSlot != i && !isDragSplit)
        {
            HandleDrop(i);
            dragHandled = true;
        }

        // Drop ke slot yang sama = cancel drag
        if (isHovered && mouseReleased && dragSlot == i)
        {
            dragSlot = -1;
            dragItem = {-1, 0};
            dragHandled = true;
        }

        HandleSplitDragSlot(i, slotRect, mousePos);
    }

    HandleSplitRelease();

    if (mouseReleased && dragSlot != -1 && !isDragSplit && !dragHandled)
    {
        InventoryItem &src = GetItemBySlotIndex(dragSlot);
        Vector2 playerCenter = PlayerInstance.GetCenter();
        Vector2 mouseWorld = GetScreenToWorld2D(mousePos, camera);
        Vector2 aimDir = Vector2Normalize(Vector2Subtract(mouseWorld, playerCenter));
        Vector2 dropPos = {
            playerCenter.x + aimDir.x * PlayerInstance.INTERACT_RANGE,
            playerCenter.y + aimDir.y * PlayerInstance.INTERACT_RANGE};

        ItemSpawn dropped = itemData.CreateItem(dropPos, dragItem.definitionId);
        dropped.amount = dragItem.amount;
        itemData.activeItems.push_back(dropped);
        src = {-1, 0};

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
    bool mousePressed = InputInstance.IsLeftClickPressed();
    bool mouseReleased = InputInstance.IsLeftClickReleased();

    for (int i = 0; i < PlayerInstance.MaxHotbar; i++)
    {
        Rectangle slotRect = {startX + (i * (slotSize + padding)), startY, slotSize, slotSize};
        bool isActive = (activeSlot == (int)(i + 1));
        int globalIdx = PlayerInstance.MaxBag + i;
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
            if (isHovered && mousePressed && dragSlot == -1 && !isDragSplit && item.definitionId != -1)
            {
                dragSlot = globalIdx;
                dragItem = item;
            }

            if (isHovered && mouseReleased && dragSlot != -1 && dragSlot != globalIdx && !isDragSplit)
                HandleDrop(globalIdx);

            HandleSplitDragSlot(globalIdx, slotRect, mousePos);
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