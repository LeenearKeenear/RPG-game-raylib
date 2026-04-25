/**
 * @file animation.cpp
 * @brief Implementasi dari Animation System & Tile Rendering
 *
 * Handle semua animasi sprite untuk player, enemy, dan entity lain.
 * Implementasi dari fungsi-fungsi yang dideklarasikan di animation.h
 */

// ================================================================
// Animation System
// Handle semua animasi sprite untuk player, enemy, dan entity lain.
//
// Semua logic animasi (frame switching, timing, direction)
// dipusatin di sini biar gak nyebar ke mana-mana.
//
// Module ini juga contain:
// - Tile texture loading & rendering (LoadTileTexture, RenderTilePNG)
// - Player animation state machine (walk, idle, attack, dead)
// - Sprite frame management (GetFrame)
// ================================================================
#include "../include/animation.h"

/*==============================================================================
 * Global Variables
 *==============================================================================*/

/** Global texture array - index sesuai TextureAsset enum */
Texture2D TexturesMap[MAX_TEXTURES];

/*==============================================================================
 * Texture Loading
 *==============================================================================*/

// ================================================================
// LoadTileTexture()
// Load texture PNG ke slot TextureAsset yang ditentuin.
// Slot ini dipake sama RenderTilePNG() buat milih texture yang bener.
// ================================================================
void LoadTileTexture(TextureAsset Slot, const char *Path)
{
    Image img = LoadImage(Path);
    TexturesMap[Slot] = LoadTextureFromImage(img);
    UnloadImage(img);
}

/*==============================================================================
 * Tile Rendering
 *==============================================================================*/

// ================================================================
// RenderTilePNG()
// Render satu tile dari spritesheet ke posisi world.
//
// Cara kerja:
// 1. Lookup TileProperty berdasarkan Type — dapet koordinat di spritesheet
// 2. Hitung Source rectangle dari koordinat itu
// 3. DrawTexturePro ke posisi Destination di world
// ================================================================
void RenderTilePNG(int pos_x, int pos_y, TileType Type, float Rotation, TextureAsset Slot)
{
    // mapping TileType ke koordinat di spritesheet
    // NOTE: Koordinat ini berdasarkan layout spritesheet tileset
    TileDefinition TileProperty[] = {
        [TILE_PLAYER_NEW] = {{3, 2}, false, false},
        [TILE_ENEMY_SLIME] = {{0, 0}, false, true},
        [TILE_ENEMY_SKELETON] = {{0, 1}, false, true},
        [TILE_ENEMY_WOLF] = {{0, 2}, false, true},
        [TILE_ITEM_POTION] = {{7, 8}, false, true},
        [TILE_WEAPON] = {{6, 4}, false, true}
    };

    // hitung posisi source di spritesheet pake koordinat + ukuran tile + gap
    // Formula: X = (kolom * (TILE_SIZE + TILE_GAP))
    Rectangle Source = {
        (float)(TileProperty[Type].CoordID.x * (TILE_SIZE + TILE_GAP)),
        (float)(TileProperty[Type].CoordID.y * (TILE_SIZE + TILE_GAP)),
        (float)TILE_SIZE,
        (float)TILE_SIZE};

    Rectangle Destination = {(float)pos_x, (float)pos_y, (float)TILE_SIZE, (float)TILE_SIZE};
    Vector2 origin = {0, 0};
    DrawTexturePro(TexturesMap[Slot], Source, Destination, origin, Rotation, WHITE);
}

/*==============================================================================
 * Tile Rendering for small sprites
 *==============================================================================*/

// ================================================================
// DrawSmallSprite()
// Render satu tile dari spritesheet ke posisi world dengan size yang lebih kecil dari tile pada umumnya.
//
// Cara kerja:
// 1. Lookup TileProperty berdasarkan Type — dapet koordinat di spritesheet
// 2. Hitung Source rectangle dari koordinat itu
// 3. DrawTexturePro ke posisi Destination di world
// ================================================================
void DrawSmallSprite(TextureAsset slot, Vector2 sheetCoord, Vector2 worldPos, float scale) {
    // 1. Source (Ambil potongan dari spritesheet)
    Rectangle source = {
        sheetCoord.x * (TILE_SIZE + TILE_GAP),
        sheetCoord.y * (TILE_SIZE + TILE_GAP),
        (float)TILE_SIZE,
        (float)TILE_SIZE
    };

    // 2. Destination (Scaling & Centering)
    float smallSize = TILE_SIZE * scale;
    float offset = (TILE_SIZE - smallSize) / 2.0f;

    Rectangle dest = {
        worldPos.x + offset,
        worldPos.y + offset,
        smallSize,
        smallSize
    };

    // 3. Render
    // Gunakan TexturesMap yang sudah ada di animation.cpp atau extern-kan
    extern Texture2D TexturesMap[]; 
    DrawTexturePro(TexturesMap[slot], source, dest, (Vector2){0,0}, 0.0f, WHITE);
}

// ================================================================
// GetFrame()
// Ambil source rectangle dari spritesheet berdasarkan frame koordinat.
// Dipakai oleh DrawPlayer() buat milih frame yang bener.
// ================================================================
Rectangle GetFrame(int frameX, int frameY)
{
    return {
        (float)(frameX * (TILE_SIZE + TILE_GAP)),
        (float)(frameY * (TILE_SIZE + TILE_GAP)),
        (float)TILE_SIZE,
        (float)TILE_SIZE};
}

/*==============================================================================
 * State Setters
 *==============================================================================*/

// ================================================================
// State Setters
// Set direction dan state animasi player.
// Dipanggil dari input handler / AI controller.
// ================================================================

void UpdatePlayerWalkUp(AnimationPlayer &p)
{
    p.direction = UP;
    p.state = WALK;
}
void UpdatePlayerWalkDown(AnimationPlayer &p)
{
    p.direction = DOWN;
    p.state = WALK;
}
void UpdatePlayerWalkLeft(AnimationPlayer &p)
{
    p.direction = LEFT;
    p.state = WALK;
}
void UpdatePlayerWalkRight(AnimationPlayer &p)
{
    p.direction = RIGHT;
    p.state = WALK;
}
void UpdatePlayerIdle(AnimationPlayer &p) { p.state = IDLE; }

void UpdatePlayerAttack(AnimationPlayer &p)
{
    if (!p.isAttacking)
    {
        p.state = ATTACK;
        p.frame = 0;
        p.frameTime = 0;
        p.isAttacking = true;
    }
}

void UpdatePlayerDeath(AnimationPlayer &p)
{
    if (!p.isDead)
    {
        p.state = DEAD;
        p.isDead = true;
        p.frame = 0;
        p.isAttacking = false; // reset attack logic if dying during attack
    }
}

/*==============================================================================
 * Animation Update Logic
 *==============================================================================*/

// ================================================================
// UpdateAnimation()
// Update frame animasi berdasarkan delta time dan current state.
//
// State machine:
// - DEAD: frame tetap 0, tidak ada animasi
// - IDLE: 2 frame, speed lambat (0.5s)
// - WALK: 4 frame cycle (0-2-0-3), speed sedang (0.15s)
// - ATTACK: jumlahnya 2 frame sequential frames
// ================================================================
void UpdateAnimation(AnimationPlayer &p, float dt)
{
    // DEAD state: gak ada animasi, frame tetap di 0
    if (p.state == DEAD)
    {
        p.frame = 0;
        return;
    }

    p.frameTime += dt;

    // IDLE state: 2 frame bolak-balik (0 -> 1 -> 0 -> 1 ...)
    if (p.state == IDLE)
    {
        p.frameSpeed = 0.5f;
        if (p.frameTime >= p.frameSpeed)
        {
            p.frame = (p.frame + 1) % 2;
            p.frameTime = 0;
        }
    }
    // WALK state: 4 frame dengan pola 0 -> 2 -> 0 -> 3
    else if (p.state == WALK)
    {
        p.frameSpeed = 0.15f;
        if (p.frameTime >= p.frameSpeed)
        {
            p.walkFrameIndex = (p.walkFrameIndex + 1) % 4;
            int walkFrames[4] = {0, 2, 0, 3}; // pola frame buat animasi jalan
            p.frame = walkFrames[p.walkFrameIndex];
            p.frameTime = 0;
        }
    }
    // ATTACK state: 2 frame sequential, pas frame ke-1 trigger "HIT!"
    else if (p.state == ATTACK)
    {
        p.frameSpeed = 0.15f;

        if (p.frameTime >= p.frameSpeed)
        {
            p.frame++;
            p.frameTime = 0;

            int maxFrames = 2;
            int hitFrame = 1; // frame dimana damage di-trigger

            if (p.frame == hitFrame)
            {
                TraceLog(LOG_INFO, "HIT!");
            }

            // attack selesai, balik ke IDLE
            if (p.frame >= maxFrames)
            {
                p.isAttacking = false;
                p.state = IDLE;
                p.frame = 0;
            }
        }
    }
}

/*==============================================================================
 * Player Rendering
 *==============================================================================*/

// ================================================================
// DrawPlayer()
// Render player sprite berdasarkan current state dan direction.
//
// Layout spritesheet knight.png:
// - Row 0-3: direction (LEFT, RIGHT, DOWN, UP)
// - Col 0-3: idle/walk frames
// - Col 4+:  attack frames
// - Row 4:   death frame (col 0)
// ================================================================
void DrawPlayer(AnimationPlayer &p)
{
    // tentuin row berdasarkan direction atau state DEAD
    int row = (int)p.direction;
    int frameX = p.frame;

    // DEAD state: pake row 4 (khusus mati)
    if (p.state == DEAD)
    {
        row = 4;
        frameX = 0;
    }
    // ATTACK state: geser frameX ke kolom attack (offset 4 atau 6)
    else if (p.state == ATTACK)
    {
        // UP/DOWN attack mulai dari kolom 4, LEFT/RIGHT dari kolom 6
        // (karena layout spritesheet punya attack frames di posisi berbeda)
        int attackOffset = (p.direction == UP || p.direction == DOWN) ? 4 : 6;
        frameX += attackOffset;
    }

    Rectangle src = GetFrame(frameX, row);

    // render pake TexturesMap[TEXTURE_KNIGHT] supaya konsisten
    // dengan LoadTileTexture yang dipake di Player::Init()
    DrawTextureRec(TexturesMap[TEXTURE_KNIGHT], src, p.position, WHITE);
}
