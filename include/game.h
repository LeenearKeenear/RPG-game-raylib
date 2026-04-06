#pragma once
#include "../lib/raylib/include/raylib.h"
#include "player.h"

struct Game
{
    int baseWidth = 1280;
    int baseHeight = 720;

    RenderTexture2D target;
    Texture2D tileset;
    Player player;
    Camera2D camera = {0};

    float zoom = 1.0f;
    float zoomMin = 0.5f;
    float zoomMax = 2.0f;

    static const int MAP_SIZE = 20;
    int tileSize = 32;

    void Init()
    {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(baseWidth, baseHeight, "RPG - Tahap 4");
        SetTargetFPS(60);

        target = LoadRenderTexture(baseWidth, baseHeight);
        tileset = LoadTexture("texture/colored_tilemap.png");

        // setup camera
        camera.target = player.position;
        camera.offset = {baseWidth / 2.0f, baseHeight / 2.0f};
        camera.rotation = 0.0f;
        camera.zoom = zoom;
    }

    void Update()
    {
        float dt = GetFrameTime();

        // player movement
        player.Update(dt);
        player.ClampToMap(MAP_SIZE, MAP_SIZE, tileSize);

        // update camera follow
        camera.target = player.position;

        // zoom control
        float wheel = GetMouseWheelMove();
        if (wheel != 0)
        {
            zoom += wheel * 0.1f;
            if (zoom < zoomMin)
                zoom = zoomMin;
            if (zoom > zoomMax)
                zoom = zoomMax;
        }
        camera.zoom = zoom;
    }

    void Draw()
    {
        BeginTextureMode(target);
        ClearBackground(DARKGRAY);

        BeginMode2D(camera);

        // ===== TILEMAP =====
        Rectangle tileSrc;

        for (int y = 0; y < MAP_SIZE; y++)
        {
            for (int x = 0; x < MAP_SIZE; x++)
            {
                if (x == 0 && y == 0) {
                    tileSrc = {0 * 36, 0 * 36, 32, 32};
                } else if (x > 0 && x < MAP_SIZE - 1 && (y == 0 || y == MAP_SIZE - 1)) {
                    tileSrc = {1 * 36, 0 * 36, 32, 32};
                } else if (x == MAP_SIZE - 1 && y == 0) {
                    tileSrc = {3 * 36, 0 * 36, 32, 32};
                } else if (x == 0 && y > 0 && y < MAP_SIZE - 1) {
                    tileSrc = {0 * 36, 1 * 36, 32, 32};
                } else if (x == MAP_SIZE - 1 && y > 0 && y < MAP_SIZE - 1) {
                    tileSrc = {3 * 36, 1 * 36, 32, 32};
                } else if (x == MAP_SIZE - 1 && y == MAP_SIZE - 1) {
                    tileSrc = {3 * 36, 2 * 36, 32, 32};
                } else if (x == 0 && y == MAP_SIZE - 1) {
                    tileSrc = {0 * 36, 2 * 36, 32, 32};
                } else if (x == MAP_SIZE - 1 && y == MAP_SIZE - 1) {
                    tileSrc = {3 * 36, 2 * 36, 32, 32};
                } else {
                    tileSrc = {4 * 36, 4 * 36, 32, 32};
                }
                Vector2 pos = {x * (float)tileSize, y * (float)tileSize};
                DrawTextureRec(tileset, tileSrc, pos, WHITE);
            }
        }

        // ===== PLAYER =====
        player.Draw(tileset);

        EndMode2D();

        DrawText("Arrow Keys = Move | Scroll = Zoom", 20, 20, 20, WHITE);

        EndTextureMode();

        // ===== DRAW TO WINDOW (16:9) =====
        BeginDrawing();
        ClearBackground(BLACK);

        float scale = GetScale();
        Vector2 offset = GetOffset(scale);

        DrawTexturePro(
            target.texture,
            {0, 0, (float)target.texture.width, -(float)target.texture.height},
            {offset.x, offset.y, baseWidth * scale, baseHeight * scale},
            {0, 0},
            0.0f,
            WHITE);

        EndDrawing();
    }

    void Close()
    {
        UnloadTexture(tileset);
        UnloadRenderTexture(target);
        CloseWindow();
    }

private:
    float GetScale()
    {
        float scaleX = (float)GetScreenWidth() / baseWidth;
        float scaleY = (float)GetScreenHeight() / baseHeight;
        return (scaleX < scaleY) ? scaleX : scaleY;
    }

    Vector2 GetOffset(float scale)
    {
        float width = baseWidth * scale;
        float height = baseHeight * scale;

        float offsetX = (GetScreenWidth() - width) / 2.0f;
        float offsetY = (GetScreenHeight() - height) / 2.0f;

        return {offsetX, offsetY};
    }
};