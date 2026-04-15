#include "../include/player_ui.h"
#include "../include/player.h"
#include "../include/screen.h"
#include "../include/map.h"
#include <raylib.h>

void PlayerUI::RenderHUD(Player* player)
{
    const int slotSize = 56;
    const int slotGap = 6;
    const int totalWidth = 4 * slotSize + 3 * slotGap;
    const int startX = (GameScreenWidth - totalWidth) / 2;
    const int startY = GameScreenHeight - slotSize - 16;

    for (int i = 0; i < 4; i++)
    {
        int x = startX + i * (slotSize + slotGap);
        bool isSelected = (i == player->SelectedHotbarSlot);

        DrawRectangle(x, startY, slotSize, slotSize, Fade(BLACK, 0.75f));

        if (isSelected)
            DrawRectangleLinesEx({(float)x, (float)startY, (float)slotSize, (float)slotSize}, 3.0f, GOLD);
        else
            DrawRectangleLines(x, startY, slotSize, slotSize, Fade(WHITE, 0.4f));

        DrawText(TextFormat("%d", i + 1), x + 4, startY + 2, 14, isSelected ? GOLD : GRAY);

        Color typeColor = player->Hotbar[i].type == SLOT_WEAPON ? SKYBLUE : LIME;
        const char *typeLabel = player->Hotbar[i].type == SLOT_WEAPON ? "W" : "P";
        DrawText(typeLabel, x + slotSize / 2 - 5, startY + slotSize / 2 - 10, 20, typeColor);

        int nameW = MeasureText(player->Hotbar[i].name, 10);
        DrawText(player->Hotbar[i].name, x + (slotSize - nameW) / 2, startY + slotSize + 2, 10,
                 isSelected ? WHITE : Fade(WHITE, 0.5f));
    }

    if (player->bInventoryOpen)
        RenderInventoryUI(player);

    if (player->bMapOpen)
        RenderMapUI(player);

    if (!player->bIsAlive)
        RenderDeathOverlay(player);
}

void PlayerUI::RenderInventoryUI(Player* player)
{
    const int panelW = 400;
    const int panelH = 350;
    const int panelX = (GameScreenWidth - panelW) / 2;
    const int panelY = (GameScreenHeight - panelH) / 2;

    DrawRectangle(panelX, panelY, panelW, panelH, Fade(BLACK, 0.85f));
    DrawRectangleLinesEx({(float)panelX, (float)panelY, (float)panelW, (float)panelH}, 2.0f, GOLD);

    const char *title = "INVENTORY";
    int titleW = MeasureText(title, 24);
    DrawText(title, panelX + (panelW - titleW) / 2, panelY + 12, 24, GOLD);

    DrawLine(panelX + 10, panelY + 42, panelX + panelW - 10, panelY + 42, Fade(GOLD, 0.5f));

    const char *items[] = {"Sword", "Bow", "Health Potion x3", "Mana Potion x2", "Shield", "Key"};
    int itemCount = 6;

    for (int i = 0; i < itemCount; i++)
    {
        int itemY = panelY + 55 + i * 40;
        DrawRectangle(panelX + 15, itemY, panelW - 30, 32, Fade(WHITE, 0.05f));
        DrawRectangleLines(panelX + 15, itemY, panelW - 30, 32, Fade(WHITE, 0.15f));
        DrawText(items[i], panelX + 25, itemY + 8, 16, WHITE);

        if (i == 0)
            DrawText("[SPACE] Equip", panelX + panelW - 130, itemY + 8, 14, YELLOW);
    }

    DrawText("[I] Close    [SPACE] Equip/Unequip", panelX + 15, panelY + panelH - 28, 14, GRAY);
}

void PlayerUI::RenderMapUI(Player* player)
{
    const int panelW = 500;
    const int panelH = 400;
    const int panelX = (GameScreenWidth - panelW) / 2;
    const int panelY = (GameScreenHeight - panelH) / 2;

    DrawRectangle(panelX, panelY, panelW, panelH, Fade(BLACK, 0.85f));
    DrawRectangleLinesEx({(float)panelX, (float)panelY, (float)panelW, (float)panelH}, 2.0f, SKYBLUE);

    const char *title = "DUNGEON MAP";
    int titleW = MeasureText(title, 24);
    DrawText(title, panelX + (panelW - titleW) / 2, panelY + 12, 24, SKYBLUE);

    DrawLine(panelX + 10, panelY + 42, panelX + panelW - 10, panelY + 42, Fade(SKYBLUE, 0.5f));

    if (tilesonMap != nullptr)
    {
        int gridW = panelW - 40;
        int gridH = panelH - 80;
        int gridX = panelX + 20;
        int gridY = panelY + 52;

        float cellW = (float)gridW / tilesonMap->width;
        float cellH = (float)gridH / tilesonMap->height;

        for (int y = 0; y < tilesonMap->height; y++)
        {
            for (int x = 0; x < tilesonMap->width; x++)
            {
                float cx = gridX + x * cellW;
                float cy = gridY + y * cellH;
                DrawRectangle((int)cx, (int)cy, (int)cellW - 1, (int)cellH - 1, Fade(DARKBLUE, 0.6f));
            }
        }

        float playerTileX = player->Position.x / TILE_SIZE;
        float playerTileY = player->Position.y / TILE_SIZE;
        float dotX = gridX + playerTileX * cellW;
        float dotY = gridY + playerTileY * cellH;
        DrawCircle((int)dotX, (int)dotY, 4.0f, GOLD);
    }
    else
    {
        DrawText("Map data not loaded", panelX + 20, panelY + 80, 18, RED);
    }

    DrawText("[M] Close", panelX + 15, panelY + panelH - 28, 14, GRAY);
}

void PlayerUI::RenderDeathOverlay(Player* player)
{
    DrawRectangle(0, 0, GameScreenWidth, GameScreenHeight, Fade(BLACK, 0.7f));
    const char *deadText = "YOU DIED";
    int textW = MeasureText(deadText, 48);
    DrawText(deadText, (GameScreenWidth - textW) / 2, GameScreenHeight / 2 - 50, 48, MAROON);
    
    const char *hintText = "Press [R] to Revive (Test)";
    int hintW = MeasureText(hintText, 20);
    DrawText(hintText, (GameScreenWidth - hintW) / 2, GameScreenHeight / 2 + 20, 20, Fade(WHITE, 0.6f));
}
