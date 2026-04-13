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

// #include "../include/animation.h"

// // --- GET FRAME FROM SPRITESHEET ---
// Rectangle GetFrame(int frameX, int frameY) {
//     return {
//         (float)(frameX * (TILE_SIZE + TILE_GAP)),
//         (float)(frameY * (TILE_SIZE + TILE_GAP)),
//         (float)TILE_SIZE,
//         (float)TILE_SIZE
//     };
// }

// // --- UPDATE PLAYER INPUT ---
// void UpdatePlayer(Player &p) {
//     // Dead = no input at all
//     if (p.isDead) return;

//     bool moving = false;

//     if (IsKeyDown(KEY_W)) {
//         p.position.y -= 2;
//         p.direction = UP;
//         moving = true;
//     }
//     if (IsKeyDown(KEY_S)) {
//         p.position.y += 2;
//         p.direction = DOWN;
//         moving = true;
//     }
//     if (IsKeyDown(KEY_A)) {
//         p.position.x -= 2;
//         p.direction = LEFT;
//         moving = true;
//     }
//     if (IsKeyDown(KEY_D)) {
//         p.position.x += 2;
//         p.direction = RIGHT;
//         moving = true;
//     }

//     // Attack input
//     if (IsKeyPressed(KEY_SPACE) && !p.isAttacking) {
//         p.state = ATTACK;
//         p.frame = 0;
//         p.frameTime = 0;
//         p.isAttacking = true;
//         return;
//     }

//     // Example: press K to "die"
//     if (IsKeyPressed(KEY_K)) {
//         p.state = DEAD;
//         p.isDead = true;
//         p.frame = 0;
//         return;
//     }

//     if (p.isAttacking) return;

//     if (moving) p.state = WALK;
//     else p.state = IDLE;
// }

// // --- UPDATE ANIMATION ---
// void UpdateAnimation(Player &p, float dt) {
//     // DEAD = no animation (single frame)
//     if (p.state == DEAD) {
//         p.frame = 0;
//         return;
//     }

//     p.frameTime += dt;

//     if (p.state == IDLE) {
//         p.frameSpeed = 0.5f;

//         if (p.frameTime >= p.frameSpeed) {
//             p.frame = (p.frame + 1) % 2;
//             p.frameTime = 0;
//         }
//     }

//     else if (p.state == WALK) {
//         p.frameSpeed = 0.15f;

//         if (p.frameTime >= p.frameSpeed) {
//             p.walkFrameIndex = (p.walkFrameIndex + 1) % 4;
//             int walkFrames[4] = {1, 2, 1, 3};
//             p.frame = walkFrames[p.walkFrameIndex];
//             p.frameTime = 0;
//         }
//     }

//     else if (p.state == ATTACK) {
//         p.frameSpeed = 0.2f;

//         if (p.frameTime >= p.frameSpeed) {
//             p.frame++;
//             p.frameTime = 0;

//             // ⚔️ Hit moment (delay)
//             if (p.frame == 1) {
//                 TraceLog(LOG_INFO, "HIT!");
//             }

//             if (p.frame >= 2) {
//                 p.isAttacking = false;
//                 p.state = IDLE;
//                 p.frame = 0;
//             }
//         }
//     }
// }

// // --- DRAW PLAYER ---
// void DrawPlayer(Player &p, Texture2D texture) {
//     int row = (int)p.direction;
//     int frameX = p.frame;

//     // Death uses row 4, frame 0
//     if (p.state == DEAD) {
//         row = 4;
//         frameX = 0;
//     }

//     Rectangle src = GetFrame(frameX, row);
//     DrawTextureRec(texture, src, p.position, WHITE);
// }