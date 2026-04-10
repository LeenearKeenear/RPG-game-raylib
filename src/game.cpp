#include "../include/game.h"

void Game::Open()
{
    Screen.Init();
    Render.Init();

    Tileset.LoadMap("texture/map.png");
    Tileset.LoadChar("texture/map.png");

    Map.Init(&Tileset);
    Player.Init(&Map);
    
    Camera.target = Player.GetPosition();
    Camera.offset = {Screen::GAME_WIDTH / 2.0f, Screen::GAME_HEIGHT / 2.0f};
    Camera.rotation = 0.0f;
    Camera.zoom = 1.0f;

    DeadZone = {
        Screen::GAME_WIDTH / 2.0f - 100,
        Screen::GAME_HEIGHT / 2.0f - 80,
        200,
        160
    };
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
    Vector2 playerPos = Player.GetPosition();
    Vector2 screenPlayerPos = GetWorldToScreen2D(playerPos, Camera);

    if (screenPlayerPos.x < DeadZone.x)
        Camera.target.x -= (DeadZone.x - screenPlayerPos.x);
    else if (screenPlayerPos.x > DeadZone.x + DeadZone.width)
        Camera.target.x += (screenPlayerPos.x - (DeadZone.x + DeadZone.width));

    if (screenPlayerPos.y < DeadZone.y)
        Camera.target.y -= (DeadZone.y - screenPlayerPos.y);
    else if (screenPlayerPos.y > DeadZone.y + DeadZone.height)
        Camera.target.y += (screenPlayerPos.y - (DeadZone.y + DeadZone.height));

    // Zoom
    if (IsKeyDown(KEY_Q)) Camera.zoom += 0.02f;
    if (IsKeyDown(KEY_E)) Camera.zoom -= 0.02f;

    if (Camera.zoom < 0.5f) Camera.zoom = 0.5f;
    if (Camera.zoom > 3.0f) Camera.zoom = 3.0f;

    // 🔒 WORLD CLAMP
    float mapWidth = Map.GetWidth() * Map.GetTileSize();
    float mapHeight = Map.GetHeight() * Map.GetTileSize();

    if (mapWidth < Screen::GAME_WIDTH / Camera.zoom)
        Camera.target.x = mapWidth / 2;

    if (mapHeight < Screen::GAME_HEIGHT / Camera.zoom)
        Camera.target.y = mapHeight / 2;

    float halfW = (Screen::GAME_WIDTH * 0.5f) / Camera.zoom;
    float halfH = (Screen::GAME_HEIGHT * 0.5f) / Camera.zoom;

    if (Camera.target.x < halfW) Camera.target.x = halfW;
    if (Camera.target.y < halfH) Camera.target.y = halfH;

    if (Camera.target.x > mapWidth - halfW)
        Camera.target.x = mapWidth - halfW;

    if (Camera.target.y > mapHeight - halfH)
        Camera.target.y = mapHeight - halfH;
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