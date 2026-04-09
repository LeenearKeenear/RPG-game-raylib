#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/screen.h"
#include "../include/Map.h"
#include "../include/player.h"

Camera2D camera = {0};
Entity Player;
sTile Door;

// movement player + collisionnya
void PlayerMovement(void)
{
    Player.MoveTimer += GetFrameTime(); // ngambil frame sekarang

    // player movement
    float PlayerPosition_x = Player.PlayerPosition.x;
    float PlayerPostition_y = Player.PlayerPosition.y;

    if (Player.MoveTimer >= Player.MoveDelay)
    {
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        {
            PlayerPosition_x -= 1 * TILE_WIDTH;
        }
        else if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        {
            PlayerPosition_x += 1 * TILE_WIDTH;
        }
        else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        {
            PlayerPostition_y -= 1 * TILE_HEIGHT;
        }
        else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        {
            PlayerPostition_y += 1 * TILE_HEIGHT;
        }

        // mapbounds dari data Map aktif
        Rectangle MapBounds = {
            0.0f,
            0.0f,
            (float)CurrentMap->TileWidth * TILE_WIDTH,
            (float)CurrentMap->TileHeight * TILE_HEIGHT,
        };

        // ngasih player collison box sendiri dengan ukuran 32 x 32 pixel
        Rectangle PlayerCollisionBox = {
            PlayerPosition_x,
            PlayerPostition_y,
            (float)TILE_WIDTH,
            (float)TILE_HEIGHT,
        };

        // cek collision
        if (CheckCollisionRecs(PlayerCollisionBox, MapBounds))
        {
            Player.PlayerPosition.x = PlayerPosition_x;
            Player.PlayerPosition.y = PlayerPostition_y;
        }

        Player.MoveTimer = 0.0f;
    }
}

// buat control player selain movement kaya interaksi, open inventory dll
void PlayerControl(void)
{
    // interaksi player
    if (IsKeyPressed(KEY_E))
    {
        // sementara variabelnya
        if (Player.PlayerPosition.x == Door.CoordinateTile.x && Player.PlayerPosition.y == Door.CoordinateTile.y)
        {
            /* code */
        }
    }
}

void PlayerCamera(void)
{
    float Maxzoom = 3.5f;
    float MinZoom = 0.85f;
    const float ZoomIncrement = 0.250f;

    float MouseWheel = GetMouseWheelMove();
    if (MouseWheel != 0)
    {
        camera.zoom += (MouseWheel * ZoomIncrement);
        if (camera.zoom > Maxzoom)
            camera.zoom = Maxzoom;
        if (camera.zoom < MinZoom)
            camera.zoom = MinZoom;
    }

    camera.target.x = (float)Player.PlayerPosition.x + (TILE_WIDTH / 2.0f);
    camera.target.y = (float)Player.PlayerPosition.y + (TILE_HEIGHT / 2.0f);

    float halfW = (GameScreenWidth / 2.0f) / camera.zoom;
    float halfH = (GameScreenHeight / 2.0f) / camera.zoom;
    float MapW = (float)CurrentMap->TileWidth * TILE_WIDTH;
    float MapH = (float)CurrentMap->TileHeight * TILE_HEIGHT;

    if (camera.target.x < halfW)
        camera.target.x = halfW;
    if (camera.target.y < halfH)
        camera.target.y = halfH;
    if (camera.target.x > MapW - halfW)
        camera.target.x = MapW - halfW;
    if (camera.target.y > MapH - halfH)
        camera.target.y = MapH - halfH;
}

// update player behavior
void UpdatePlayer(GameState *state)
{
    PlayerMovement();
    PlayerControl();
    PlayerCamera();
}