#pragma once
#include "../lib/raylib/include/raylib.h"
#include "Player.h"
#include "Map.h"

struct Game
{
    int BaseWidth = 1280;
    int BaseHeight = 720;

    RenderTexture2D Dungeon;
    Camera2D Camera = {0};
    Texture2D TileMap;
    Texture2D TileChar;
    Player Player;
    Map Map;

    float Zoom = 1.5f;
    float ZoomMin = 1.0f;
    float ZoomMax = 3.0f;

    void Init()
    {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(BaseWidth, BaseHeight, "Dungeon Game");
        SetTargetFPS(60);

        Dungeon = LoadRenderTexture(BaseWidth, BaseHeight);

        TileMap = LoadTexture("texture/map.png");
        TileChar = LoadTexture("texture/character.png");

        Map.Init();

        Camera.offset = {
            (float)(BaseWidth / 2),
            (float)(BaseHeight / 2)
        };
        Camera.target = {
            Player.Position.x - Player.Size / 2.0f,
            Player.Position.y - Player.Size / 2.0f};
    }

    void Update()
    {
        float Dt = GetFrameTime();

        Player.Update(Dt, Map);

        Camera.target = Player.Position;

        float Wheel = GetMouseWheelMove();
        if (Wheel != 0)
        {
            Zoom += Wheel * 0.1f;
            if (Zoom < ZoomMin)
                Zoom = ZoomMin;
            if (Zoom > ZoomMax)
                Zoom = ZoomMax;
        }

        Camera.zoom = Zoom;
    }

    void Draw()
    {
        BeginTextureMode(Dungeon);
        ClearBackground(BLACK);

        BeginMode2D(Camera);

        Map.Draw(TileMap);
        Player.Draw(TileChar);

        EndMode2D();

        DrawText("Arrow = Move | Scroll = Zoom", 20, 20, 20, WHITE);

        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);

        float Scale = GetScale();
        Vector2 Offset = GetOffset(Scale);

        DrawTexturePro(
            Dungeon.texture,
            {0, 0, (float)Dungeon.texture.width, -(float)Dungeon.texture.height},
            {Offset.x, Offset.y, BaseWidth * Scale, BaseHeight * Scale},
            {0, 0},
            0.0f,
            WHITE);

        EndDrawing();
    }

    void Close()
    {
        UnloadTexture(TileMap);
        UnloadTexture(TileChar);
        UnloadRenderTexture(Dungeon);
        CloseWindow();
    }

private:
    float GetScale()
    {
        float ScaleX = (float)GetScreenWidth() / BaseWidth;
        float ScaleY = (float)GetScreenHeight() / BaseHeight;
        return (ScaleX < ScaleY) ? ScaleX : ScaleY;
    }

    Vector2 GetOffset(float Scale)
    {
        float Width = BaseWidth * Scale;
        float Height = BaseHeight * Scale;

        return {
            (GetScreenWidth() - Width) / 2.0f,
            (GetScreenHeight() - Height) / 2.0f};
    }
};