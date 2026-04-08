#include "../include/game.h"

void Game::Init()
{
    Screen.Init();
    Render.Init();
    // entry point untuk inisialisasi
}

void Game::Run()
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
    // entry point untuk update
}

void Game::Draw()
{
    Render.Begin();

    // entry point untuk render
    Debug.Mouse(Screen);

    Render.End();
    Render.Draw(Screen);
}

void Game::Shutdown()
{
    Render.Shutdown();
    Screen.Shutdown();
}