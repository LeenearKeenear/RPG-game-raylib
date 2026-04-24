/**
 * @file entities.cpp
 * @brief Implementasi dari Entities Coordination Module
 *
 * Implementasi dari fungsi-fungsi yang dideklarasikan di entities.h
 * Handle master rendering semua entity dalam game.
 */

#include "../include/entities.h"
#include "../include/player.h"
#include "../include/enemy.h"
#include "../include/item.h"

/*==============================================================================
 * Public Functions
 *==============================================================================*/

// ================================================================
// RenderEntities()
// Master render semua entity, dipanggil dalam BeginMode2D block
// Urutan render: item → enemy → player (nanti)
// ================================================================
void RenderEntities(void)
{
    // TODO: RenderItems();    // nanti tambahin pas item udah diimplementasi
    RenderAllItems();
    // Render player terlebih dahulu (paling bawah/layer terbawah)
    PlayerInstance.Render();
    // TODO: RenderEnemies();  // nanti tambahin pas enemy udah diimplementasi
    RenderAllEnemies();
}