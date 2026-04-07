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
            0,
            0,
            CurrentMap->TileWidth * TILE_WIDTH,
            CurrentMap->TileHeight * TILE_HEIGHT,
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

// buat setup camera
void PlayerCamera(void)
{
    float Maxzoom = 3.5f;  // maksimal zoom in
    float MinZoom = 0.85f; // minimal zoom out

    // ukuran kotak deadzone dalam pixel
    int SizeDeadZone_x = 300;
    int SizeDeadZone_y = 200;

    // buat zoom multipliernya. makin gede makin cepet zoomnya
    const float ZoomIncrement = 0.250f;

    // inisialisasi deadzonennya
    Rectangle DeadZone = {SizeDeadZone_x, SizeDeadZone_y, SizeDeadZone_x, SizeDeadZone_y};

    // fungsi biar bisa ngezoom via mousewheel
    float MouseWheel = GetMouseWheelMove();
    if (MouseWheel != 0)
    {
        camera.zoom += (MouseWheel * ZoomIncrement);
        if (camera.zoom > Maxzoom)
            camera.zoom = Maxzoom;
        if (camera.zoom < MinZoom)
            camera.zoom = MinZoom;
    }

    // konversi posisi player dari world ke screen
    Vector2 ScreenPoint = GetWorldToScreen2D(
        (Vector2){(float)Player.PlayerPosition.x, (float)Player.PlayerPosition.y},
        camera);

    // cek apakah player keluar deadzone, baru gerakin kamera
    if (ScreenPoint.x < DeadZone.x)
        camera.target.x -= DeadZone.x - ScreenPoint.x;
    if (ScreenPoint.x > GameScreenWidth - DeadZone.width)
        camera.target.x += ScreenPoint.x - (GameScreenWidth - DeadZone.width);
    if (ScreenPoint.y < DeadZone.y)
        camera.target.y -= DeadZone.y - ScreenPoint.y;
    if (ScreenPoint.y > GameScreenHeight - DeadZone.height)
        camera.target.y += ScreenPoint.y - (GameScreenHeight - DeadZone.height);

    // clamp biar gak keliatan area putih
    float halfW = (GameScreenWidth / 2.0f) / camera.zoom;
    float halfH = (GameScreenHeight / 2.0f) / camera.zoom;

    // data buat clamp kamera berdasarkan data size current map
    float MapW = CurrentMap->TileWidth * TILE_WIDTH;
    float MapH = CurrentMap->TileHeight * TILE_HEIGHT;

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