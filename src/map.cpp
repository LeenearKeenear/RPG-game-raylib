#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/screen.h"
#include "../include/Map.h"

// definisi awal currentmap harus nullptr
MapDataDefinition *CurrentMap = nullptr;

Texture2D TexturesMap[MAX_TEXTURES];

Camera2D camera = {0};
Entity Player;
sTile Door;

// definisi tile property sama id gambarnya (bisa berubah tergantung kalian)
TileDefinition TileProperty[] = {
    [TILE_CLU_WALL] = {{0, 0}, false, false},
    [TILE_CMU_WALL] = {{1, 0}, false, false},
    [TILE_CRU_WALL] = {{3, 0}, false, false},
    [TILE_CML_WALL] = {{0, 1}, false, false},
    [TILE_M_WALL] = {{1, 1}, false, false},
    [TILE_CMR_WALL] = {{3, 1}, false, false},
    [TILE_CLD_WALL] = {{0, 2}, false, false},
    [TILE_CMD_WALL] = {{1, 2}, false, false},
    [TILE_CRD_WALL] = {{3, 2}, false, false},
    [TILE_POOL] = {{12, 8}, false, false},
    [TILE_BIGMAN] = {{7, 0}, false, false},
    [TILE_GRASS1] = {{4, 4}, true, false},
    [TILE_GRASS2] = {{5, 4}, true, false},
    [TILE_DOOR_OPEN] = {{4, 2}, true, true},
    [TILE_DOOR_CLOSE] = {{5, 2}, false, true},
};

// buat ngeload texture dari berbagai png
void LoadTileTexture(TextureAsset Slot, const char *Path)
{
    Image img = LoadImage(Path);
    TexturesMap[Slot] = LoadTextureFromImage(img);
    UnloadImage(img);
}

// buat ngerender tile dari gambar png nya
void RenderTilePNG(int pos_x, int pos_y, TileType Type, float Rotation, TextureAsset Slot)
{
    // buat ngetrack indexing dari gambar png nya
    Rectangle Source = {(float)(TileProperty[Type].CoordID.x * (TILE_WIDTH + TILE_GAP)), (float)(TileProperty[Type].CoordID.y * (TILE_HEIGHT + TILE_GAP)),
                        (float)TILE_WIDTH, (float)TILE_HEIGHT};
    Rectangle Destination = {(float)(pos_x), (float)(pos_y),
                             (float)TILE_WIDTH, (float)TILE_HEIGHT};
    Vector2 origin = {0, 0};
    DrawTexturePro(TexturesMap[Slot], Source, Destination, origin, Rotation, WHITE);
}

void DebugMenu(float NewX, float NewY)
{
    // debug camera
    DrawRectangle(5, 5, 330, 120, RED);
    DrawRectangleLines(5, 5, 330, 120, WHITE);
    DrawText(TextFormat("kamera target: (%06.2f, %06.2f)", camera.target.x, camera.target.y), 15, 10, 25, YELLOW);
    DrawText(TextFormat("kamera zoom: %06.2f", camera.zoom), 15, 30, 25, YELLOW);

    // debug collision
    // TODO MULTI-MAP: MapBounds harus ngambil dari data Map aktif, bukan WORLD_WIDTH/HEIGHT langsung
    Rectangle MapBounds = {0, 0, WORLD_WIDTH * TILE_WIDTH, WORLD_HEIGHT * TILE_HEIGHT};
    Rectangle PlayerCollisionBox = {NewX, NewY, (float)TILE_WIDTH, (float)TILE_HEIGHT};
    bool hit = CheckCollisionRecs(PlayerCollisionBox, MapBounds);

    DrawRectangle(5, 130, 330, 100, DARKGRAY);
    DrawRectangleLines(5, 130, 330, 100, WHITE);
    DrawText("-- DEBUG COLLISION --", 15, 135, 25, YELLOW);
    DrawText(TextFormat("Player Pos: (%.0f, %.0f)", NewX / TILE_WIDTH, NewY / TILE_HEIGHT), 15, 155, 25, WHITE);
    DrawText(TextFormat("Map Bounds: (%d x %d)", (CurrentMap->TileWidth * TILE_WIDTH) / TILE_WIDTH, (CurrentMap->TileHeight * TILE_HEIGHT) / TILE_HEIGHT), 15, 175, 25, WHITE);
    DrawText(TextFormat("CheckCollisionRecs: %s", hit ? "TRUE" : "FALSE"), 15, 195, 25, hit ? GREEN : RED);
}

// movement player + collisionnya (sementara collisonnya)
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

        // TODO MULTI-MAP: MapBounds harus dari data Map aktif
        Rectangle MapBounds = {
            0,
            0,
            CurrentMap->TileWidth * TILE_WIDTH,
            CurrentMap->TileHeight * TILE_HEIGHT,
        };

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

    int SizeDeadZone_x = 300;
    int SizeDeadZone_y = 200;

    // buat zoom multipliernya. makin gede makin cepet zoomnya
    const float ZoomIncrement = 0.250f;

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

    // TODO MULTI-MAP: MapW dan MapH harus dari ukuran Map aktif
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

// fungsi buat loadmap dan ini masih prototipe tester
// semua data level map lewat entry point ini
void LoadMap(void)
{
    MapDataDefinition *Map = new MapDataDefinition();
    Map->TileWidth = 20;
    Map->TileHeight = 20;
    Map->SpawnPointPlayer = {0, 0};

    // alokasi 2D array dinamis
    Map->Tiles = new sTile *[Map->TileWidth];
    for (int i = 0; i < Map->TileWidth; i++)
    {
        Map->Tiles[i] = new sTile[Map->TileHeight];
        for (int j = 0; j < Map->TileHeight; j++)
        {
            Map->Tiles[i][j] = (sTile){
                .CoordinateTile = {i, j},
                .type = (TileType)GetRandomValue(TILE_GRASS1, TILE_GRASS2)}; // bagian ini bisa digamti, tapi karena masih tester gini dulu
        }
    }

    CurrentMap = Map;
}

// buat hapus buffer array 2d nya kalo udah gak kepake
void UnloadMap()
{
    if (CurrentMap == nullptr)
        return;

    for (int i = 0; i < CurrentMap->TileWidth; i++)
    {
        delete[] CurrentMap->Tiles[i];
    }

    delete[] CurrentMap->Tiles;
    delete CurrentMap;
    CurrentMap = nullptr;
}

// TODO MULTI-MAP: fungsi ini harus nerima parameter Map mana yang mau di-load
// bukan hardcode ke WorldMap
// inisialisasi Map dalam bentuk data (handle Map)
void InitDrawMap(GameState *state)
{
    LoadTileTexture(TEXTURE_TILEMAP, "texture/colored_tileMap.png");
    LoadMap();
}

// render Mapnya berdasarkan data (handle Map)
void RenderMap(GameState *state)
{
    BeginMode2D(camera);

    sTile tile;

    for (int i = 0; i < CurrentMap->TileWidth; i++)
    {
        for (int j = 0; j < CurrentMap->TileHeight; j++)
        {
            tile = CurrentMap->Tiles[i][j];

            // buat ngetrack indexing dari gambar png nya
            RenderTilePNG((tile.CoordinateTile.x * TILE_WIDTH), (tile.CoordinateTile.y * TILE_HEIGHT), tile.type, 0, TEXTURE_TILEMAP);
        }
    }
    // sementara
    RenderTilePNG(Door.CoordinateTile.x, Door.CoordinateTile.y, TILE_DOOR_OPEN, 0.0, TEXTURE_TILEMAP);

    RenderTilePNG(Player.PlayerPosition.x, Player.PlayerPosition.y, TILE_BIGMAN, 0.0, TEXTURE_TILEMAP); // disini playernya

    EndMode2D();
    DebugMenu(Player.PlayerPosition.x, Player.PlayerPosition.y);
}
