/**
 * @file entities.cpp
 * @brief Implementasi dari Entities Coordination Module
 *
 * Implementasi dari fungsi-fungsi yang dideklarasikan di entities.h
 * Handle master rendering semua entity dalam game.
 */

#include "../include/entities.h"
#include "../include/player.h"

/*==============================================================================
 * Public Functions
 *==============================================================================*/

// ================================================================
// RenderEntities()
// Master render semua entity, dipanggil dalam BeginMode2D block
// Urutan render: player → enemy → item (nanti)
// ================================================================
void RenderEntities(void)
{
    // Render player terlebih dahulu (paling bawah/layer terbawah)
    PlayerInstance.Render();

    // TODO: RenderEnemies();  // nanti tambahin pas enemy udah diimplementasi
    // TODO: RenderItems();    // nanti tambahin pas item udah diimplementasi
}