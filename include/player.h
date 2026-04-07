#pragma once
<<<<<<< HEAD
#include <cmath>
#include "../lib/raylib/include/raylib.h"
#include "map.h"

enum Direction {
    DOWN,
    UP,
    RIGHT,
    LEFT
};

struct Player
{
    Vector2 Position = {200, 200};
    Direction Direction = DOWN;

    static const int TILE_SIZE = 32;
    float Speed = 150.0f;
    float FrameTime = 0;
    float FrameSpeed = 0.1f;

    // update posisi player berdasarkan input, delta time, dan map
    void Update(float DeltaTime, Map &Map)
    {
        Vector2 Movement = {0, 0};

        // mengambil input arah untuk menentukan movement dan arah player
        if (IsKeyDown(KEY_RIGHT))
        {
            Movement.x += 1;
            Direction = RIGHT;
        }
        if (IsKeyDown(KEY_LEFT))
        {
            Movement.x -= 1;
            Direction = LEFT;
        }
        if (IsKeyDown(KEY_DOWN))
        {
            Movement.y += 1;
            Direction = DOWN;
        }
        if (IsKeyDown(KEY_UP))
        {
            Movement.y -= 1;
            Direction = UP;
        }

        // cek apakah player bergerak atau tidak
        if (Movement.x != 0 || Movement.y != 0)
        {
            // normalisasi movement agar kecepatan diagonal tidak lebih cepat
            float Len = sqrt(Movement.x * Movement.x + Movement.y * Movement.y);
            Movement.x /= Len;
            Movement.y /= Len;
        }
        else
        {
            // jika tidak bergerak, jangan update posisi dan arah
            Direction = DOWN;
            return;
        }

        Vector2 NewPosition = {
            Position.x + Movement.x * Speed * DeltaTime,
            Position.y + Movement.y * Speed * DeltaTime};

        // simpan nilai setiap ujung sudut player
        int Left = (int)(NewPosition.x / Map.TILE_SIZE);
        int Right = (int)((NewPosition.x + TILE_SIZE - 1) / Map.TILE_SIZE);
        int Top = (int)(NewPosition.y / Map.TILE_SIZE);
        int Bottom = (int)((NewPosition.y + TILE_SIZE - 1) / Map.TILE_SIZE);

        // cek apakah keempat sudut player tidak menabrak tile bertipe blocked
        if (!Map.IsBlocked(Left, Top) &&
            !Map.IsBlocked(Right, Top) &&
            !Map.IsBlocked(Left, Bottom) &&
            !Map.IsBlocked(Right, Bottom))
        {
            Position = NewPosition;
        }
    }

    // akan digunakan untuk animasi berjalan
    void Animate(float DeltaTime) {}

    // menggambar player ke render texture
    void Draw(Texture2D TileChar)
    {
        Rectangle Source = {Direction * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE};
        Rectangle Destination = {Position.x, Position.y, (float)TILE_SIZE, (float)TILE_SIZE};

        DrawTexturePro(TileChar, Source, Destination, {0, 0}, 0.0f, WHITE);
    }
};
=======
#include "../lib/raylib/include/raylib.h"
#include "../include/screen.h"
#include "../include/map.h"

// TODO MULTI-MAP: Entity nanti bakal butuh info
// di map mana dia berada (map id / pointer ke map aktif)
// sementara doang (ini buat entity (contoh player, enemy, npc))
typedef struct
{
    TileCoordinate PlayerPosition;
    float MoveTimer;
    float MoveDelay;
} Entity;

// definisi struct entity ama tile khusus
extern Entity Player;
extern sTile Door;

// gak penting
void PlayerMovement(void);
void PlayerControl(void);
void PlayerCamera(void);

// penting
void UpdatePlayer(GameState *state);
>>>>>>> main
