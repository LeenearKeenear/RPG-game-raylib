#pragma once
#include "../lib/raylib/include/raylib.h"
#include "Player.h"
#include "Map.h"

struct Game
{
    static const int BASE_WIDTH = 1280;
    static const int BASE_HEIGHT = 720;

    RenderTexture2D Dungeon;
    Texture2D TileMap;
    Texture2D TileChar;

    Player Player;
    Map Map;

    Camera2D Camera = {0};

    static const float ZOOM_MIN = 1.5f;
    static const float ZOOM_MAX = 4.0f;
    float Zoom = 2.5f;

    // inisialisasi window, kamera, dan load texture
    void Init()
    {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE); // agar window bisa diubah ukurannya
        InitWindow(BASE_WIDTH, BASE_HEIGHT, "Dungeon Game"); // ukuran dan judul window
        SetTargetFPS(60);

        // load texture dan render texture
        Dungeon = LoadRenderTexture(BASE_WIDTH, BASE_HEIGHT);
        TileMap = LoadTexture("texture/map.png");
        TileChar = LoadTexture("texture/character.png");

        Map.Init();

        // mengatur offset dan posisi kamera agar player selalu berada di tengah layar
        Camera.offset = {(BASE_WIDTH / 2.0f) - 32.0f, (BASE_HEIGHT / 2.0f) - 32.0f};
        Camera.target = Player.Position;
    }

    // update player, kamera, dan zoom
    void Update()
    {
        // mendapatkan delta time untuk update player
        float DeltaTime = GetFrameTime();
        Player.Update(DeltaTime, Map);

        // setelah player terupdate, baru update kamera agar mengikuti player
        Camera.target = Player.Position;

        // mengambil input scroll untuk zoom in dan zoom out
        float Wheel = GetMouseWheelMove();
        if (Wheel != 0)
        {
            Zoom += Wheel * 0.1f;
            if (Zoom < ZOOM_MIN)
                Zoom = ZOOM_MIN;
            if (Zoom > ZOOM_MAX)
                Zoom = ZOOM_MAX;
        }

        Camera.zoom = Zoom;
    }

    // menggambar map dan player ke render texture, lalu dilanjutkan ke window
    void Draw()
    {
        // menggambar ke render texture terlebih dahulu
        BeginTextureMode(Dungeon);
        ClearBackground(BLACK);

        // menggambar map dan player dengan kamera yang sudah diatur
        BeginMode2D(Camera);

        Map.Draw(TileMap);
        Player.Draw(TileChar);

        EndMode2D();

        EndTextureMode();

        // menggambar render texture ke window dengan scaling dan offset
        BeginDrawing();
        ClearBackground(BLACK);

        float Scale = GetScale();
        Vector2 Offset = GetOffset(Scale);

        // menggambar render texture dengan scaling dan offset
        DrawTexturePro(
            Dungeon.texture,
            {0, 0, (float)Dungeon.texture.width, -(float)Dungeon.texture.height},
            {Offset.x, Offset.y, BASE_WIDTH * Scale, BASE_HEIGHT * Scale},
            {0, 0},
            0.0f,
            WHITE);

        EndDrawing();
    }

    // unload semua texture dan render texture, lalu tutup window
    void Close()
    {
        UnloadTexture(TileMap);
        UnloadTexture(TileChar);
        UnloadRenderTexture(Dungeon);
        CloseWindow();
    }

private:
    // menentukan scale ukuran render texture yang sesuaidengan ukuran window
    float GetScale()
    {
        float ScaleX = (float)GetScreenWidth() / BASE_WIDTH;
        float ScaleY = (float)GetScreenHeight() / BASE_HEIGHT;
        return (ScaleX < ScaleY) ? ScaleX : ScaleY;
    }

    // menentukan offset agar render texture tetap di tengah jika ukuran window berubah
    Vector2 GetOffset(float Scale)
    {
        float Width = BASE_WIDTH * Scale;
        float Height = BASE_HEIGHT * Scale;

        return {
            (GetScreenWidth() - Width) / 2.0f,
            (GetScreenHeight() - Height) / 2.0f};
    }
};