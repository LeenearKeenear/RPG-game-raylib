#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/screen.h"
#include "../include/map.h"

// ukuran tile buat di mapping sprite nya
#define TILE_WIDTH 32
#define TILE_HEIGHT 32
#define TILE_GAP 0

// ukuran map berdasarkan tilesnya (jadi (20 x TILE_WIDTH) x (20 x TILE_HEIGHT))
#define WORLD_WIDTH 20
#define WORLD_HEIGHT 20

sTile WorldMap[WORLD_WIDTH][WORLD_HEIGHT];

Texture2D TexturesMap[MAX_TEXTURES];

Camera2D camera = {0};

// inisialisasi map dalam bentuk data
void InitDrawMap(GameState *state)
{
    /*
    buat ngeload gambar tilemapnya. gambarnya itu ukuran 128 x 80 terus dibagi sama 8 x 8,
    jadinya dapet 16 set gambar kesamping sama 10 set gambar kebawah (ini masih experimen)
    */
    Image TileMap = LoadImage("texture/colored_tilemap.png");
    TexturesMap[TEXTURE_TILEMAP] = LoadTextureFromImage(TileMap);
    UnloadImage(TileMap);

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

    // inisialisasi camera
    camera.target = (Vector2){0, 0};
    camera.offset = (Vector2){(float)(GameScreenWidth / 2), (float)(GameScreenHeight / 2)};
    camera.rotation = {0};
    camera.zoom = 1.0f;
}

// update map
void UpdateMap(GameState *state)
{

    // fungsi biar bisa ngezoom via mousewheel
    float MouseWheel = GetMouseWheelMove();
    if (MouseWheel != 0)
    {
        const float ZoomIncrement = 0.125f;
        camera.zoom += (MouseWheel * ZoomIncrement);
    }

    // buat selalu update posisi kameranya berdasarkan player
    camera.target = (Vector2){0, 0};
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
            TextureIndex_X = 7;
            TextureIndex_Y = 0;

            // buat ngetrack indexing dari gambar png nya
            Rectangle Source = {(float)(TextureIndex_X * (TILE_WIDTH + TILE_GAP)), (float)(TextureIndex_Y * (TILE_HEIGHT + TILE_GAP)),
                                (float)TILE_WIDTH, (float)TILE_HEIGHT};
            Rectangle Destination = {(float)(tile.x * TILE_WIDTH), (float)(tile.y * TILE_HEIGHT),
                                     (float)TILE_WIDTH, (float)TILE_HEIGHT};
            Vector2 origin = {0, 0};
            DrawTexturePro(TexturesMap[TEXTURE_TILEMAP], Source, Destination, origin, zoom, WHITE);
        }
    }
    EndMode2D();
}