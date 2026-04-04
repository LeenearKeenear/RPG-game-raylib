#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/screen.h"
#include "../include/map.h"

// ukuran map berdasarkan tilesnya (jadi (20 x TILE_WIDTH) x (20 x TILE_HEIGHT))
#define WORLD_WIDTH 20
#define WORLD_HEIGHT 20

sTile WorldMap[WORLD_WIDTH][WORLD_HEIGHT];

Texture2D TexturesMap[MAX_TEXTURES];

Camera2D camera = {0};
Entity Player;

// buat ngeload texture dari berbagai png
void LoadTileTexture(TextureAsset Slot, const char *Path)
{
    Image img = LoadImage(Path);
    TexturesMap[Slot] = LoadTextureFromImage(img);
    UnloadImage(img);
}

// buat ngerender tile dari gambar png nya
void RenderTilePNG_1Tile(int pos_x, int pos_y, int TextureIndex_X, int TextureIndex_Y, float Rotation, TextureAsset Slot)
{
    // buat ngetrack indexing dari gambar png nya
    Rectangle Source = {(float)(TextureIndex_X * (TILE_WIDTH + TILE_GAP)), (float)(TextureIndex_Y * (TILE_HEIGHT + TILE_GAP)),
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

    DrawText(TextFormat("kamera target: (%06.2f, %06.2f)", camera.target.x, camera.target.y), 15, 10, 14, YELLOW);
    DrawText(TextFormat("kamera zoom: %06.2f", camera.zoom), 15, 30, 14, YELLOW);
}

// inisialisasi map dalam bentuk data
void InitDrawMap(GameState *state)
{

    LoadTileTexture(TEXTURE_TILEMAP, "texture/colored_tilemap.png");

    // inisialisasi tile dalam bentuk array 2d
    for (int i = 0; i < WORLD_WIDTH; i++)
    {
        for (int j = 0; j < WORLD_HEIGHT; j++)
        {
            WorldMap[i][j] = (sTile){
                .x = i,
                .y = j,
            };
        }
    }
}

// update map
void UpdateMap(GameState *state)
{

    float Maxzoom = 8.0f; // maksimal zoom in
    float MinZoom = 1.0f; // minimal zoom out

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

    // buat selalu update posisi kameranya berdasarkan player
    camera.target = (Vector2){Player.x, Player.y};
}

// render mapnya berdasarkan data
void RenderMap(GameState *state)
{
    BeginMode2D(camera);

    sTile tile;

    // index buat nge track gambar png nya mulai dari [0][0]
    int TextureIndex_X = 0;
    int TextureIndex_Y = 0;

    float zoom = 0.0f; // buat ukuran zoomnya

    for (int i = 0; i < WORLD_WIDTH; i++)
    {
        for (int j = 0; j < WORLD_HEIGHT; j++)
        {
            tile = WorldMap[i][j];

            // index buat gambar grass
            TextureIndex_X = 12;
            TextureIndex_Y = 8;

            // buat ngetrack indexing dari gambar png nya
            Rectangle Source = {(float)(TextureIndex_X * (TILE_WIDTH + TILE_GAP)), (float)(TextureIndex_Y * (TILE_HEIGHT + TILE_GAP)),
                                (float)TILE_WIDTH, (float)TILE_HEIGHT};
            Rectangle Destination = {(float)(tile.x * TILE_WIDTH), (float)(tile.y * TILE_HEIGHT),
                                     (float)TILE_WIDTH, (float)TILE_HEIGHT};
            Vector2 origin = {0, 0};
            DrawTexturePro(TexturesMap[TEXTURE_TILEMAP], Source, Destination, origin, zoom, WHITE);
        }
    }

    RenderTilePNG_1Tile(camera.target.x, camera.target.y, 7, 0, 0.0, TEXTURE_TILEMAP);
    EndMode2D();
    DebugMenu();
}
