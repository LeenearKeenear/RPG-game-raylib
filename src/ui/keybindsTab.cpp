/**
 * @file keybindsTab.cpp
 * @brief Implementasi Keybinds Settings Tab
 * 
 * Handle rendering untuk daftar keybinds.
 */

#include "keybindsTab.h"
#include "fonts.h"
#include "../lib/raylib/include/raylib.h"
#include <array>
#include <algorithm>

struct KeybindEntry {
    const char* key;
    const char* action;
};

static const std::array<KeybindEntry, 13> mainKeybinds = {{
    {"W / Arrow Up", "Move Up"},
    {"S / Arrow Down", "Move Down"},
    {"A / Arrow Left", "Move Left"},
    {"D / Arrow Right", "Move Right"},
    {"E", "Interact"},
    {"I", "Inventory"},
    {"M", "Map"},
    {"Q", "Drop Item"},
    {"Left Ctrl", "Drop All"},
    {"Mouse Left", "Attack"},
    {"Mouse Right", "Dash / Drink"},
    {"1 / 2 / 3 / 4", "Hotbar Slots"},
    {"Scroll", "Hotbar Slot"}
}};

static const std::array<KeybindEntry, 5> debugKeybinds = {{
    {"`", "Pause Menu"},
    {"Tab", "Debug Overlay"},
    {"R", "Revive"},
    {"K", "Damage (No Effect)"},
    {"B", "Previous Map"}
}};

/**
 * @brief Draw keybinds tab content
 * @param mousePosition Posisi mouse untuk efek hover
 * @param startX Posisi X awal area options
 * @param startY Posisi Y awal area options
 */
void DrawKeybindsTab(Vector2 mousePosition, int startX, int startY) {
    (void)mousePosition;

    static int scrollY = 0;

    int contentStartY = startY + 100;
    const int headerHeight = 36;
    const int rowHeight = 24;
    const int colX = startX + 40;
    const int keyColWidth = 220;
    const int sepWidth = 30;
    const int availableHeight = 420;

    // 2 headers (72px) + 18 entries (432px) + 1 gap (24px) = 528px total
    int totalContentHeight = 2 * headerHeight +
        (static_cast<int>(mainKeybinds.size()) + static_cast<int>(debugKeybinds.size()) + 1) * rowHeight;
    int maxScroll = std::max(0, totalContentHeight - availableHeight);

    scrollY -= static_cast<int>(GetMouseWheelMove()) * 24;
    scrollY = std::clamp(scrollY, 0, maxScroll);

    // Convert a local Y offset (from contentStartY) to screen Y (accounting for scroll)
    auto screenY = [&](int localY) -> int {
        return contentStartY + localY - scrollY;
    };

    // Check if an element at localY with given height falls within the visible area
    auto isVisible = [&](int localY, int height) -> bool {
        int top = screenY(localY);
        int bottom = top + height;
        int viewTop = contentStartY - headerHeight;
        int viewBottom = contentStartY + availableHeight + headerHeight;
        return bottom > viewTop && top < viewBottom;
    };

    // Current local Y offset from contentStartY (before scrolling)
    int currentY = 0;

    // ---- "=== MAIN ===" header ----
    if (isVisible(currentY, headerHeight)) {
        DrawTextEx(fontKeybindHeader, "=== MAIN ===",
            Vector2{(float)colX, (float)screenY(currentY)},
            22, 0, YELLOW);
    }
    currentY += headerHeight;

    // ---- Main keybinds ----
    for (const auto& entry : mainKeybinds) {
        if (isVisible(currentY, rowHeight)) {
            int y = screenY(currentY);
            DrawTextEx(fontKeybindEntry, entry.key,
                Vector2{(float)colX, (float)y}, 20, 0, YELLOW);
            DrawTextEx(fontKeybindEntry, "=>",
                Vector2{(float)(colX + keyColWidth), (float)y}, 20, 0, GRAY);
            DrawTextEx(fontKeybindEntry, entry.action,
                Vector2{(float)(colX + keyColWidth + sepWidth), (float)y}, 20, 0, WHITE);
        }
        currentY += rowHeight;
    }

    currentY += rowHeight;

    // ---- "=== DEBUGGING ===" header ----
    if (isVisible(currentY, headerHeight)) {
        DrawTextEx(fontKeybindHeader, "=== DEBUGGING ===",
            Vector2{(float)colX, (float)screenY(currentY)},
            22, 0, GRAY);
    }
    currentY += headerHeight;

    // ---- Debug keybinds ----
    for (const auto& entry : debugKeybinds) {
        if (isVisible(currentY, rowHeight)) {
            int y = screenY(currentY);
            DrawTextEx(fontKeybindEntry, entry.key,
                Vector2{(float)colX, (float)y}, 20, 0, YELLOW);
            DrawTextEx(fontKeybindEntry, "=>",
                Vector2{(float)(colX + keyColWidth), (float)y}, 20, 0, GRAY);
            DrawTextEx(fontKeybindEntry, entry.action,
                Vector2{(float)(colX + keyColWidth + sepWidth), (float)y}, 20, 0, WHITE);
        }
        currentY += rowHeight;
    }

    // ---- Scroll indicators ----
    if (maxScroll > 0) {
        int indicatorX = startX + 350;
        if (scrollY > 0) {
            DrawTextEx(fontKeybindEntry, "^^^",
                Vector2{(float)indicatorX, (float)(contentStartY - 5)},
                16, 0, GRAY);
        }
        if (scrollY < maxScroll) {
            DrawTextEx(fontKeybindEntry, "vvv",
                Vector2{(float)indicatorX, (float)(contentStartY + availableHeight - 20)},
                16, 0, GRAY);
        }
    }
}
