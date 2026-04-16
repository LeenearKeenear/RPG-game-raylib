#pragma once

/**
 * @file entities.h
 * @brief Entities Coordination Module
 *
 * Coordinator buat semua entity di game (player, enemy, item).
 * Jadi tempat sentral buat ngelola render dan update semua entity.
 */

/*==============================================================================
 * Entities Functions
 *==============================================================================*/

/**
 * @brief Master render buat semua entity
 * @note Dipanggil di dalam BeginMode2D block
 *       Handle render player, enemy, item, dll dalam satu tempat
 *
 * Roadmap kedepan:
 * - Spawn logic enemy/item bakal masuk sini
 * - Update logic enemy/item bakal masuk sini
 */
void RenderEntities(void);