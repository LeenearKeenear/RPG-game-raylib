#include "../include/game.h"

void Game::Open()
{
    Screen.Init();
    Render.Init();

    Tileset.LoadMap("texture/map.png");
    Tileset.LoadChar("texture/map.png");

    Map.Init(&Tileset);
    Player.Init(&Map);
    
    Camera.target = {0, 0};
    Camera.offset = {Screen::GAME_WIDTH / 2.0f, Screen::GAME_HEIGHT / 2.0f};
    Camera.rotation = 0.0f;
    Camera.zoom = 1.0f;
}

void Game::Loop()
{
    while (!WindowShouldClose())
    {
        Update();
        Draw();
    }
}

void Game::Update()
{
    Screen.Update();
    Player.Update();
    Camera.target = Player.GetPosition();

    // Zoom control
    if (IsKeyDown(KEY_Q)) Camera.zoom += 0.02f;
    if (IsKeyDown(KEY_E)) Camera.zoom -= 0.02f;

    if (Camera.zoom < 0.5f) Camera.zoom = 0.5f;
    if (Camera.zoom > 3.0f) Camera.zoom = 3.0f;
}

void Game::Draw()
{
    Render.Begin();

    BeginMode2D(Camera);
    Map.Render();
    Player.Render();
    EndMode2D();

    Debug.Mouse(Screen);
    Render.End();

    Render.Draw(Screen);
}

void Game::Close()
{
    Render.Shutdown();
    Screen.Shutdown();
}