#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/screen.h"
#include "../include/Map.h"
#include "../include/player.h"

// definisi awal currentmap harus nullptr
MapDataDefinition *CurrentMap = nullptr;

// Texture2D TexturesMap[MAX_TEXTURES];

// definisi tile property sama id gambarnya (bisa berubah tergantung kalian)
// sama ini keknya dipindah di file baru aja yang isinya nge handle definisi tile
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
// sama ini keknya dipindah di file baru aja yang isinya nge handle definisi tile
void LoadTileTexture(TextureAsset Slot, const char *Path)
{
    Image img = LoadImage(Path);
    TexturesMap[Slot] = LoadTextureFromImage(img);
    UnloadImage(img);
}

// buat ngerender tile dari gambar png nya
// sama ini keknya dipindah di file baru aja yang isinya nge handle definisi tile
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

// fungsi buat debug menu
void DebugMenu(float NewX, float NewY)
{
    // debug camera
    DrawRectangle(5, 5, 330, 120, RED);
    DrawRectangleLines(5, 5, 330, 120, WHITE);
    DrawText(TextFormat("kamera target: (%06.2f, %06.2f)", camera.target.x, camera.target.y), 15, 10, 25, YELLOW);
    DrawText(TextFormat("kamera zoom: %06.2f", camera.zoom), 15, 30, 25, YELLOW);

    // debug collision
    Rectangle MapBounds = {0, 0, CurrentMap->TileWidth * TILE_WIDTH, CurrentMap->TileHeight * TILE_HEIGHT};
    Rectangle PlayerCollisionBox = {NewX, NewY, (float)TILE_WIDTH, (float)TILE_HEIGHT};
    bool hit = CheckCollisionRecs(PlayerCollisionBox, MapBounds);

    DrawRectangle(5, 130, 330, 100, DARKGRAY);
    DrawRectangleLines(5, 130, 330, 100, WHITE);
    DrawText("-- DEBUG COLLISION --", 15, 135, 25, YELLOW);
    DrawText(TextFormat("Player Pos: (%.0f, %.0f)", NewX / TILE_WIDTH, NewY / TILE_HEIGHT), 15, 155, 25, WHITE);
    DrawText(TextFormat("Map Bounds: (%d x %d)", (CurrentMap->TileWidth * TILE_WIDTH) / TILE_WIDTH, (CurrentMap->TileHeight * TILE_HEIGHT) / TILE_HEIGHT), 15, 175, 25, WHITE);
    DrawText(TextFormat("CheckCollisionRecs: %s", hit ? "TRUE" : "FALSE"), 15, 195, 25, hit ? GREEN : RED);
}

// fungsi buat loadmap dan ini masih prototipe tester
// semua data level map lewat entry point ini (handle map)
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
                .type = (TileType)GetRandomValue(TILE_GRASS1, TILE_GRASS2)};
            // bagian ini bisa digamti, tapi karena masih tester gini dulu
        }
    }

    CurrentMap = Map;
}

// buat hapus buffer array 2d nya kalo udah gak kepake (handle map)
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

//     sTile tile;

    // kayaknya ini fungsinya sementara deh
    // tapi jangan di hapus dulu sampe bisa ngambil data desain level dalam bentuk json atau apapun itu kesini
    for (int i = 0; i < CurrentMap->TileWidth; i++)
    {
        for (int j = 0; j < CurrentMap->TileHeight; j++)
        {
            tile = CurrentMap->Tiles[i][j];

//             // buat ngetrack indexing dari gambar png nya
//             RenderTilePNG((tile.CoordinateTile.x * TILE_WIDTH), (tile.CoordinateTile.y * TILE_HEIGHT), tile.type, 0, TEXTURE_TILEMAP);
//         }
//     }
//     // sementara
//     RenderTilePNG(Door.CoordinateTile.x, Door.CoordinateTile.y, TILE_DOOR_OPEN, 0.0, TEXTURE_TILEMAP);

//     RenderTilePNG(Player.PlayerPosition.x, Player.PlayerPosition.y, TILE_BIGMAN, 0.0, TEXTURE_TILEMAP); // disini playernya

//     EndMode2D();
//     DebugMenu(Player.PlayerPosition.x, Player.PlayerPosition.y);
// }
