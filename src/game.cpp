#include "../include/game.h"

void Game::Open()
{
    Screen.Init();
    Render.Init();
    Map.Init();
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
}

void Game::Draw()
{
    Render.Begin();

    Map.Render();
    Debug.Mouse(Screen);

    Render.End();
    Render.Draw(Screen);
}

void Game::Close()
{
    Render.Shutdown();
    Screen.Shutdown();
}