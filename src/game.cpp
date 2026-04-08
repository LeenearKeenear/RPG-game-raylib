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
    Render.TextureMode(Screen);
    Render.Drawing(Screen);
}

void Game::Shutdown()
{
    Render.Shutdown();
    Screen.Shutdown();
}