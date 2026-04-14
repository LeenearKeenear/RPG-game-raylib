#include "../include/entities.h"
#include "../include/player.h"
#include "../include/enemy.h"

// ================================================================
// RenderEntities()
// Master render semua entity, dipanggil dalam BeginMode2D block
// Urutan render: player → enemy → item (nanti)
// ================================================================
void RenderEntities(void)
{
    PlayerInstance.Render();
    // TODO: RenderEnemies();
    EnemyRenderTest();
    // TODO: RenderItems();
}