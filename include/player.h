#pragma once
#include <cmath>
#include "../lib/raylib/include/raylib.h"
#include "Map.h"

struct Player
{
    Vector2 Position = {200, 200};
    float Speed = 150.0f;
    int Size = 32;

    int Frame = 0;
    float FrameTime = 0;
    float FrameSpeed = 0.1f;

    void Update(float Dt, Map &Map)
    {
        Vector2 Movement = {0, 0};

        if (IsKeyDown(KEY_RIGHT))
        {
            Movement.x += 1;
            Frame = 2;
        }
        if (IsKeyDown(KEY_LEFT))
        {
            Movement.x -= 1;
            Frame = 3;
        }
        if (IsKeyDown(KEY_DOWN))
        {
            Movement.y += 1;
            Frame = 0;
        }
        if (IsKeyDown(KEY_UP))
        {
            Movement.y -= 1;
            Frame = 1;
        }

        if (Movement.x != 0 || Movement.y != 0)
        {
            float Len = sqrt(Movement.x * Movement.x + Movement.y * Movement.y);
            Movement.x /= Len;
            Movement.y /= Len;
        }
        else
        {
            Frame = 0;
        }

        Vector2 NewPos = {
            Position.x + Movement.x * Speed * Dt,
            Position.y + Movement.y * Speed * Dt};

        int Left = (int)(NewPos.x / Map.TILE_SIZE);
        int Right = (int)((NewPos.x + Size - 1) / Map.TILE_SIZE);
        int Top = (int)(NewPos.y / Map.TILE_SIZE);
        int Bottom = (int)((NewPos.y + Size - 1) / Map.TILE_SIZE);

        if (!Map.IsBlocked(Left, Top) &&
            !Map.IsBlocked(Right, Top) &&
            !Map.IsBlocked(Left, Bottom) &&
            !Map.IsBlocked(Right, Bottom))
        {
            Position = NewPos;
        }
    }

    void Animate(float Dt)
    {
        FrameTime += Dt;
        if (FrameTime >= FrameSpeed)
        {
            Frame++;
            if (Frame > 3)
                Frame = 0;
            FrameTime = 0;
        }
    }

    void Draw(Texture2D Texture)
    {
        Rectangle Src = {Frame * 32.0f, 0, 32, 32};
        Rectangle Dest = {Position.x, Position.y, (float)Size, (float)Size};

        DrawTexturePro(Texture, Src, Dest, {0, 0}, 0.0f, WHITE);
    }
};