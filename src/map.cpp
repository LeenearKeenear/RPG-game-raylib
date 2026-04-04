#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/screen.h"
#include "../include/map.h"

// ukuran map berdasarkan tilesnya (jadi (20 x TILE_WIDTH) x (20 x TILE_HEIGHT))
// oh ya ini masih eksperimen buat sizenya. nanti bakal diubah
#define WORLD_WIDTH 20
#define WORLD_HEIGHT 20

sTile WorldMap[WORLD_WIDTH][WORLD_HEIGHT];

Texture2D TexturesMap[MAX_TEXTURES];

Camera2D camera = {0};
Entity Player;

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

    float Maxzoom = 3.5f; // maksimal zoom in
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

    // buat selalu update posisi kameranya berdasarkan player
    camera.target = (Vector2){Player.x, Player.y};
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
            RenderTilePNG((tile.x * TILE_WIDTH), (tile.y * TILE_HEIGHT), TILE_POOL, 0, TEXTURE_TILEMAP);
        }
    }

    RenderTilePNG(camera.target.x, camera.target.y, TILE_BIGMAN, 0.0, TEXTURE_TILEMAP);
    EndMode2D();
    DebugMenu();
}
