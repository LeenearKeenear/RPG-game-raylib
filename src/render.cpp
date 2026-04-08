#include "../include/render.h"

void Render::Init()
{
    Dungeon = LoadRenderTexture(Screen::GAME_WIDTH, Screen::GAME_HEIGHT);
    SetTextureFilter(Dungeon.texture, TEXTURE_FILTER_BILINEAR);
}

void Render::Begin()
{
    BeginTextureMode(Dungeon);
    ClearBackground(BLACK);
}

void Render::End()
{
    EndTextureMode();
}

void Render::Draw(const Screen &Screen)
{
    float Scale = Screen.GetGameScale();
    float OffsetX = (Screen.GetWindowWidth() - (Screen::GAME_WIDTH * Scale)) * 0.5f;
    float OffsetY = (Screen.GetWindowHeight() - (Screen::GAME_HEIGHT * Scale)) * 0.5f;

    BeginDrawing();
    ClearBackground(BLACK);

    DrawTexturePro(
        Dungeon.texture,
        (Rectangle){0, 0, (float)Screen::GAME_WIDTH, -(float)Screen::GAME_HEIGHT},
        (Rectangle){OffsetX, OffsetY, Screen::GAME_WIDTH * Scale, Screen::GAME_HEIGHT * Scale},
        (Vector2){0, 0},
        0.0f,
        WHITE);

    EndDrawing();
}

void Render::Shutdown()
{
    UnloadRenderTexture(Dungeon);
}