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


// // ================================================================
// // LoadTileTexture()
// // Load texture PNG ke slot TextureAsset yang ditentuin.
// // Slot ini dipake sama RenderTilePNG() buat milih texture yang bener.
// // ================================================================
// // dawg ini dipindah dawg
// void LoadTileTexture(TextureAsset Slot, const char *Path)
// {
//     Image img = LoadImage(Path);
//     TexturesMap[Slot] = LoadTextureFromImage(img);
//     UnloadImage(img);
// }

// // ================================================================
// // RenderTilePNG()
// // Render satu tile dari spritesheet ke posisi world.
// //
// // Cara kerja:
// // 1. Lookup TileProperty berdasarkan Type — dapet koordinat di spritesheet
// // 2. Hitung Source rectangle dari koordinat itu
// // 3. DrawTexturePro ke posisi Destination di world
// // ================================================================
// // dawg ini dipindah dawg
// void RenderTilePNG(int pos_x, int pos_y, TileType Type, float Rotation, TextureAsset Slot)
// {
//     // mapping TileType ke koordinat di spritesheet
//     TileDefinition TileProperty[] = {
//         [TILE_CLU_WALL] = {{0, 0}, false, false},
//         [TILE_CMU_WALL] = {{1, 0}, false, false},
//         [TILE_CRU_WALL] = {{3, 0}, false, false},
//         [TILE_CML_WALL] = {{0, 1}, false, false},
//         [TILE_M_WALL] = {{1, 1}, false, false},
//         [TILE_CMR_WALL] = {{3, 1}, false, false},
//         [TILE_CLD_WALL] = {{0, 2}, false, false},
//         [TILE_CMD_WALL] = {{1, 2}, false, false},
//         [TILE_CRD_WALL] = {{3, 2}, false, false},
//         [TILE_POOL] = {{12, 8}, false, false},
//         [TILE_BIGMAN] = {{7, 0}, false, false},
//         [TILE_GRASS1] = {{4, 4}, true, false},
//         [TILE_GRASS2] = {{5, 4}, true, false},
//         [TILE_DOOR_OPEN] = {{4, 2}, true, true},
//         [TILE_DOOR_CLOSE] = {{5, 2}, false, true},
//         [TILE_PLAYER_NEW] = {{3, 2}, false, false}};

//     // hitung posisi source di spritesheet pake koordinat + ukuran tile + gap
//     Rectangle Source = {
//         (float)(TileProperty[Type].CoordID.x * (TILE_SIZE + TILE_GAP)),
//         (float)(TileProperty[Type].CoordID.y * (TILE_SIZE + TILE_GAP)),
//         (float)TILE_SIZE,
//         (float)TILE_SIZE};

//     Rectangle Destination = {(float)pos_x, (float)pos_y, (float)TILE_SIZE, (float)TILE_SIZE};
//     Vector2 origin = {0, 0};
//     DrawTexturePro(TexturesMap[Slot], Source, Destination, origin, Rotation, WHITE);
// }


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