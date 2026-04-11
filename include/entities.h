#pragma once

// ================================================================
// Entities
// Coordinator buat semua entity di game (player, enemy, item)
// - RenderEntities() jadi master render dalam satu BeginMode2D block
// - Nanti spawn & update logic enemy/item masuk sini juga
// ================================================================

void RenderEntities();