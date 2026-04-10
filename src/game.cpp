#include "../include/game.h"

void Game::Open()
{
    Screen.Init();
    Render.Init();

    Tileset.LoadMap("texture/map.png");
    Tileset.LoadChar("texture/map.png");

    Map.Init(&Tileset);
    Player.Init(&Map);
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
}

void Game::Draw()
{
    Render.Begin();

    Map.Render();
    Player.Render();
    Debug.Mouse(Screen);

    Render.End();
    Render.Draw(Screen);
}

void Game::Close()
{
    Render.Shutdown();
    Screen.Shutdown();
}