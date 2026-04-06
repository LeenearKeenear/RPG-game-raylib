#pragma once
#include "../lib/raylib/include/raylib.h"

struct Player
{
    Vector2 position = {100, 100};
    float speed = 200.0f;
    int size = 32;

    Rectangle src = {4 * 36, 0 * 36, 32, 32};

    void Update(float dt)
    {
        if (IsKeyDown(KEY_RIGHT))
            position.x += speed * dt;
        if (IsKeyDown(KEY_LEFT))
            position.x -= speed * dt;
        if (IsKeyDown(KEY_DOWN))
            position.y += speed * dt;
        if (IsKeyDown(KEY_UP))
            position.y -= speed * dt;
    }

    void ClampToMap(int mapWidth, int mapHeight, int tileSize)
    {
        float maxX = mapWidth * tileSize - size;
        float maxY = mapHeight * tileSize - size;

        if (position.x < 0)
            position.x = 0;
        if (position.y < 0)
            position.y = 0;
        if (position.x > maxX)
            position.x = maxX;
        if (position.y > maxY)
            position.y = maxY;
    }

    void Draw(Texture2D texture)
    {
        Rectangle dest = {position.x, position.y, (float)size, (float)size};
        DrawTexturePro(texture, src, dest, {0, 0}, 0.0f, WHITE);
    }
};