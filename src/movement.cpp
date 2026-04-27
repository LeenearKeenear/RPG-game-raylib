#include "../include/movement.h"
#include "../include/player.h"
#include "../include/mapLogic.h"
#include "../include/input.h"
#include "../include/tiles.h"
#include "../include/debug.h"
#include <cmath>

namespace Movement
{
    /**
     * Memperbarui kecepatan dan posisi pemain berdasarkan input.
     * Mengimplementasikan "sliding collision" untuk mencegah pemain tertahan di tembok saat bergerak diagonal.
     */
    void HandleMovement(Player &player)
    {
        player.Velocity = {0, 0};
        bool moving = false;
        Direction nextDir = player.Anim.direction;

        // Mengambil vektor input mentah
        if (InputInstance.IsMoveUp())    { player.Velocity.y -= 1; nextDir = UP;    moving = true; }
        if (InputInstance.IsMoveDown())  { player.Velocity.y += 1; nextDir = DOWN;  moving = true; }
        if (InputInstance.IsMoveLeft())  { player.Velocity.x -= 1; nextDir = LEFT;  moving = true; }
        if (InputInstance.IsMoveRight()) { player.Velocity.x += 1; nextDir = RIGHT; moving = true; }

        // Memperbarui status animasi berdasarkan status pergerakan
        ::State nextState = moving ? WALK : IDLE;
        PlayAnimation(player.Anim, nextState, nextDir, PlayerAnimationSet);

        // Normalisasi kecepatan untuk kecepatan yang konsisten di semua arah (perbaikan diagonal)
        float Length = sqrtf(player.Velocity.x * player.Velocity.x + player.Velocity.y * player.Velocity.y);
        if (Length != 0)
        {
            player.Velocity.x /= Length;
            player.Velocity.y /= Length;
        }

        // --- Logika Sliding Collision ---
        // Memeriksa sumbu X dan Y secara independen. Jika salah satu terhalang, sumbu lainnya masih bisa bergerak.
        Vector2 nextX = { player.Position.x + player.Velocity.x * player.Speed, player.Position.y };
        Vector2 nextY = { player.Position.x, player.Position.y + player.Velocity.y * player.Speed };

        if (CanMove(player, nextX))
        {
            player.Position.x = nextX.x;
        }
        if (CanMove(player, nextY))
        {
            player.Position.y = nextY.y;
        }

        player.Anim.position = player.Position;
    }

    /**
     * Memperbarui posisi dan zoom kamera.
     * Membatasi kamera pada batas map untuk mencegah melihat ke area kosong (void).
     */
    void UpdateCamera(Player &player)
    {
        float MapW = (float)tilesonMap->width * TILE_SIZE;
        float MapH = (float)tilesonMap->height * TILE_SIZE;

        // Zoom Dinamis: Perkecil zoom sedikit untuk map yang sangat kecil agar muat di layar
        const int MinMapTileZoom = 15;
        float AutoZoom = (float)GameScreenWidth / (MinMapTileZoom * TILE_SIZE);
        float FixedZoom = 2.0f;
        const float CameraZoom = (tilesonMap->width <= MinMapTileZoom || tilesonMap->height <= MinMapTileZoom)
                                     ? AutoZoom
                                     : FixedZoom;

        if (!isDebugMode)
            camera.zoom = CameraZoom;

        // Menargetkan titik tengah pemain
        camera.target.x = player.Position.x + (TILE_SIZE / 2.0f);
        camera.target.y = player.Position.y + (TILE_SIZE / 2.0f);

        // Ukuran viewport kamera dalam koordinat dunia
        float halfW = (GameScreenWidth / 2.0f) / camera.zoom;
        float halfH = (GameScreenHeight / 2.0f) / camera.zoom;

        // Membatasi target kamera pada tepi map
        if (MapW <= halfW * 2.0f)
            camera.target.x = MapW / 2.0f; // Menengahkan kamera jika map lebih kecil dari viewport
        else
        {
            if (camera.target.x < halfW)
                camera.target.x = halfW;
            if (camera.target.x > MapW - halfW)
                camera.target.x = MapW - halfW;
        }

        if (MapH <= halfH * 2.0f)
            camera.target.y = MapH / 2.0f;
        else
        {
            if (camera.target.y < halfH)
                camera.target.y = halfH;
            if (camera.target.y > MapH - halfH)
                camera.target.y = MapH - halfH;
        }
    }

    /**
     * Memvalidasi apakah posisi yang diusulkan aman untuk pemain.
     * Memeriksa batas dunia, tile tabrakan persegi panjang, dan bentuk poligon.
     */
    bool CanMove(Player &player, Vector2 newPosition)
    {
        Rectangle hitbox = BuildHitbox(newPosition, player.HitboxOffsetX, player.HitboxOffsetY, player.HitboxWidth, player.HitboxHeight);

        // 1. Pemeriksaan batas dunia (World Boundary)
        if (!IsWithinWorldBounds(hitbox, tilesonMap->width * TILE_SIZE, tilesonMap->height * TILE_SIZE))
            return false;

        // 2. Pemeriksaan layer tabrakan persegi panjang Tiled
        if (CheckCollisionAgainstRects(hitbox, player.CollisionRects))
            return false;

        // 3. Pemeriksaan tabrakan poligon/objek Tiled
        if (CheckCollisionAgainstPolygons(hitbox, player.CollisionPolygons))
            return false;

        return true;
    }
}
