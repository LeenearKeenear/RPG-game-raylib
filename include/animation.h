// #pragma once

// // ================================================================
// // Animation System
// // Handle semua animasi sprite untuk player, enemy, dan entity lain.
// //
// // Semua logic animasi (frame switching, timing, direction)
// // dipusatin di sini biar gak nyebar ke mana-mana.
// //
// // TODO (pindahan dari map.h / map.cpp):
// // - LoadTileTexture()
// // - RenderTilePNG()
// // - TileDefinition struct
// // - TileType enum
// // - TileCoordinate struct
// // - TextureAsset enum
// // - TexturesMap array
// // - MAX_TEXTURES define
// // - TILE_SIZE, TILE_GAP define
// //
// // Setelah dipindah, update semua include dan pemanggilan
// // di map.cpp, player.cpp, entities.cpp, dll.
// // ================================================================

// #include "raylib.h"

// // --- CONFIG ---
// const int TILE_SIZE = 32;
// const int TILE_GAP = 4;

// // --- ENUMS ---
// enum State {
//     IDLE,
//     WALK,
//     ATTACK,
//     DEAD
// };

// enum Direction {
//     LEFT,
//     RIGHT,
//     DOWN,
//     UP
// };

// // --- PLAYER STRUCT ---
// struct Player {
//     Vector2 position;

//     State state;
//     Direction direction;

//     int frame;
//     float frameTime;
//     float frameSpeed;

//     int walkFrameIndex;

//     bool isAttacking;
//     bool isDead;
// };

// // --- FUNCTION DECLARATIONS ---
// // Get frame rectangle from spritesheet
// Rectangle GetFrame(int frameX, int frameY);

// // Update player input and state
// void UpdatePlayer(Player &p);

// // Update animation frames based on state
// void UpdateAnimation(Player &p, float dt);

// // Draw player sprite
// void DrawPlayer(Player &p, Texture2D texture);