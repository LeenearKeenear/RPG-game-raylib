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

    // Konfigurasi layout (menggunakan konstanta dari screen_handler jika perlu, 
    // tapi sementara hardcoded sesuai desain sebelumnya)
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
}
