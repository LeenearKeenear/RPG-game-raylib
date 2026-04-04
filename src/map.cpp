#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/screen.h"
#include "../include/map.h"

// ukuran map berdasarkan tilesnya (jadi (20 x TILE_WIDTH) x (20 x TILE_HEIGHT))
// oh ya ini masih eksperimen buat sizenya. nanti bakal diubah
#define WORLD_WIDTH 30
#define WORLD_HEIGHT 30

sTile WorldMap[WORLD_WIDTH][WORLD_HEIGHT];
sTile Dungeon[WORLD_WIDTH][WORLD_HEIGHT];

Texture2D TexturesMap[MAX_TEXTURES];

Camera2D camera = {0};
Entity Player;
PropsAttributes Door;

// sementara tapi konsepnya gini
TileCoordinate TileCoords[] = {
    [TILE_CLU_WALL] = {0, 0},
    [TILE_CMU_WALL] = {1, 0},
    [TILE_CRU_WALL] = {3, 0},
    [TILE_CML_WALL] = {0, 1},
    [TILE_M_WALL] = {1, 1},
    [TILE_CMR_WALL] = {3, 1},
    [TILE_CLD_WALL] = {0, 2},
    [TILE_CMD_WALL] = {1, 2},
    [TILE_CRD_WALL] = {3, 2},
    [TILE_POOL] = {12, 8},
    [TILE_BIGMAN] = {7, 0},
    [TILE_GRASS1] = {4, 4},
    [TILE_GRASS2] = {5, 4},
    [TILE_DOOR_OPEN] = {4, 2},
    [TILE_DOOR_CLOSE] = {5, 2}};

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
    Rectangle Source = {(float)(TileCoords[Type].x * (TILE_WIDTH + TILE_GAP)), (float)(TileCoords[Type].y * (TILE_HEIGHT + TILE_GAP)),
                        (float)TILE_WIDTH, (float)TILE_HEIGHT};
    Rectangle Destination = {(float)(pos_x), (float)(pos_y),
                             (float)TILE_WIDTH, (float)TILE_HEIGHT};
    Vector2 origin = {0, 0};
    DrawTexturePro(TexturesMap[Slot], Source, Destination, origin, Rotation, WHITE);
}

// fungsi debugger
void DebugMenu()
{
    // debugger buat zoom
    DrawRectangle(5, 5, 330, 120, RED);
    DrawRectangleLines(5, 5, 330, 120, WHITE);

    DrawText(TextFormat("kamera target: (%06.2f, %06.2f)", camera.target.x, camera.target.y), 15, 10, 25, YELLOW);
    DrawText(TextFormat("kamera zoom: %06.2f", camera.zoom), 15, 30, 25, YELLOW);
}

// inisialisasi map dalam bentuk data
void InitDrawMap(GameState *state)
{
    LoadTileTexture(TEXTURE_TILEMAP, "texture/colored_tilemap.png");

    // inisialisasi tile dalam bentuk array 2d. entry point buat desain map nya juga bisa
    for (int i = 0; i < WORLD_WIDTH; i++)
    {
        for (int j = 0; j < WORLD_HEIGHT; j++)
        {
            // ini keknya sementara
            WorldMap[i][j] = (sTile){
                .x = i,
                .y = j,
                .type = (TileType)GetRandomValue(TILE_GRASS1, TILE_GRASS2)};

            Dungeon[i][j] = (sTile){
                .x = i,
                .y = j,
                .type = (TileType)TILE_GRASS2};
        }
    }
}

// update map
void UpdatePlayer(GameState *state)
{
    Player.MoveTimer += GetFrameTime(); // ngambil frame sekarang

    // player movement
    float PlayerPosition_x = Player.x;
    float PlayerPostition_y = Player.y;

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

        Player.MoveTimer = 0.0f;
    }

    float Maxzoom = 3.5f;  // maksimal zoom in
    float MinZoom = 0.85f; // minimal zoom out

    // fungsi biar bisa ngezoom via mousewheel
    float MouseWheel = GetMouseWheelMove();
    if (MouseWheel != 0)
    {
        const float ZoomIncrement = 1.0f;
        camera.zoom += (MouseWheel * ZoomIncrement);
        if (camera.zoom > Maxzoom)
            camera.zoom = Maxzoom;
        if (camera.zoom < MinZoom)
            camera.zoom = MinZoom;
    }

    Player.x = PlayerPosition_x;
    Player.y = PlayerPostition_y;

    // buat selalu update posisi kameranya berdasarkan player
    camera.target = (Vector2){Player.x, Player.y};

    // interaksi player
    if (IsKeyPressed(KEY_E))
    {
        // sementara variabelnya
        if (Player.x == Door.x && Player.y == Door.y)
        {
            /* code */
        }
    }
}

// render mapnya berdasarkan data
void RenderMap(GameState *state)
{
    BeginMode2D(camera);

    sTile tile;

    for (int i = 0; i < WORLD_WIDTH; i++)
    {
        for (int j = 0; j < WORLD_HEIGHT; j++)
        {
            tile = WorldMap[i][j];

            // buat ngetrack indexing dari gambar png nya
            RenderTilePNG((tile.x * TILE_WIDTH), (tile.y * TILE_HEIGHT), tile.type, 0, TEXTURE_TILEMAP);
        }
    }
    // sementara
    RenderTilePNG(Door.x, Door.y, TILE_DOOR_OPEN, 0.0, TEXTURE_TILEMAP);

    RenderTilePNG(camera.target.x, camera.target.y, TILE_BIGMAN, 0.0, TEXTURE_TILEMAP); // disini playernya
    EndMode2D();
    DebugMenu();
}
