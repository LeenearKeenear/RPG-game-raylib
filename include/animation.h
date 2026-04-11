#pragma once

// ================================================================
// Animation System
// Handle semua animasi sprite untuk player, enemy, dan entity lain.
//
// Semua logic animasi (frame switching, timing, direction)
// dipusatin di sini biar gak nyebar ke mana-mana.
//
// TODO (pindahan dari map.h / map.cpp):
// - LoadTileTexture()
// - RenderTilePNG()
// - TileDefinition struct
// - TileType enum
// - TileCoordinate struct
// - TextureAsset enum
// - TexturesMap array
// - MAX_TEXTURES define
// - TILE_SIZE, TILE_GAP define
//
// Setelah dipindah, update semua include dan pemanggilan
// di map.cpp, player.cpp, entities.cpp, dll.
// ================================================================