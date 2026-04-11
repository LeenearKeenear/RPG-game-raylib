#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/screen.h"
#include "../include/map.h"
#include "../include/player.h"
#include <memory>
#include <string>

TilesonMapData *tilesonMap = nullptr;
static std::unique_ptr<tson::Map> parsedMap = nullptr;

// definisi awal currentmap harus nullptr
MapDataDefinition *CurrentMap = nullptr;

Texture2D TexturesMap[MAX_TEXTURES];

Camera2D camera = {0};

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
    // definisi tile property sama id gambarnya
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
        [TILE_PLAYER_NEW] = {{3, 2}, false, false}};

    // buat ngetrack indexing dari gambar png nya
    Rectangle Source = {
        (float)(TileProperty[Type].CoordID.x * (TILE_SIZE + TILE_GAP)),
        (float)(TileProperty[Type].CoordID.y * (TILE_SIZE + TILE_GAP)),
        (float)TILE_SIZE,
        (float)TILE_SIZE};
    Rectangle Destination = {(float)pos_x, (float)pos_y, (float)TILE_SIZE, (float)TILE_SIZE};
    Vector2 origin = {0, 0};
    DrawTexturePro(TexturesMap[Slot], Source, Destination, origin, Rotation, WHITE);
}

// parsingnya
void TilesonLoadMap(const char *mapPath)
{
    tson::Tileson t;
    // parse file JSON map nya
    parsedMap = t.parse(mapPath);

    // cek apakah parsing berhasil
    if (parsedMap->getStatus() != tson::ParseStatus::OK)
    {
        TraceLog(LOG_ERROR, "Tileson: Failed to parse map: %s", parsedMap->getStatusMessage().c_str());
        return;
    }

    tilesonMap = new TilesonMapData();
    tson::Vector2i mapSize = parsedMap->getSize();
    tilesonMap->width = mapSize.x;
    tilesonMap->height = mapSize.y;

    // hitung dulu berapa layer TileLayer yang ada di map
    int layerCount = 0;
    for (auto &layer : parsedMap->getLayers())
        if (layer.getType() == tson::LayerType::TileLayer)
            layerCount++;

    // alokasi array of array, satu slot per layer
    tilesonMap->layerCount = layerCount;
    tilesonMap->tiles = new int *[layerCount];
    for (int i = 0; i < layerCount; i++)
        tilesonMap->tiles[i] = new int[tilesonMap->width * tilesonMap->height]();

    // isi data tile tiap layer
    int layerIndex = 0;
    for (auto &layer : parsedMap->getLayers())
    {
        if (layer.getType() == tson::LayerType::TileLayer)
        {
            // tileData itu map dengan key (x,y) dan value pointer ke Tile
            std::map<std::tuple<int, int>, tson::Tile *> tileData = layer.getTileData();
            for (const auto &[pos, tile] : tileData)
            {
                int x = std::get<0>(pos);
                int y = std::get<1>(pos);
                if (tile != nullptr && x < tilesonMap->width && y < tilesonMap->height)
                    // konversi posisi 2D ke index 1D, simpen di layer yang sesuai
                    tilesonMap->tiles[layerIndex][(y * tilesonMap->width) + x] = (int)tile->getId();
            }
            layerIndex++;
        }
    }

    // parse object layer
    for (auto &layer : parsedMap->getLayers())
    {
        if (layer.getType() == tson::LayerType::ObjectGroup)
        {
            for (auto &obj : layer.getObjects())
            {
                MapObject mapObj;
                mapObj.name = obj.getName();
                mapObj.type = obj.getType();

                tson::Vector2i pos = obj.getPosition();
                tson::Vector2i size = obj.getSize();
                mapObj.bounds = {(float)pos.x, (float)pos.y, (float)size.x, (float)size.y};

                // ambil semua custom properties nya
                for (auto &[key, prop] : obj.getProperties().getProperties())
                    mapObj.properties[key] = prop;

                tilesonMap->Objects.push_back(mapObj);
            }
        }
    }

    // ambil tileset pertama yang ada di map
    auto &tilesets = parsedMap->getTilesets();
    if (!tilesets.empty())
    {
        tson::Tileset *tileset = &tilesets[0];
        // ambil nama file nya aja, prefix texture/ di depan (sesuaiin sama struktur folder)
        std::string imagePath = "texture/" + tileset->getImagePath().filename().u8string();
        TraceLog(LOG_INFO, "Image path: %s", imagePath.c_str());
        Image img = LoadImage(imagePath.c_str());
        tilesonMap->tilesetTexture = LoadTextureFromImage(img);
        tilesonMap->tilesetCols = tileset->getColumns();      // jumlah kolom di tileset
        tilesonMap->tilesetSpacing = tileset->getSpacing();   // gap antar tile di image
        tilesonMap->tilesetFirstgid = tileset->getFirstgid(); // ID awal tile (biasanya 1)
        UnloadImage(img);                                     // udah jadi texture, image CPU nya bisa dibuang
        TraceLog(LOG_INFO, "Tileson: Loaded tileset texture: %s", imagePath.c_str());
    }
    else
    {
        TraceLog(LOG_WARNING, "Tileson: No tileset found in map");
    }

    TraceLog(LOG_INFO, "Tileson: Map loaded - %dx%d tiles", tilesonMap->width, tilesonMap->height);
}

// ngeload mapnya di array
void TilesonUnloadMap()
{
    if (tilesonMap != nullptr)
    {
        // hapus tiap array tile per layer dulu sebelum hapus array utamanya
        for (int i = 0; i < tilesonMap->layerCount; i++)
            delete[] tilesonMap->tiles[i];
        delete[] tilesonMap->tiles;

        tilesonMap->Objects.clear();

        // unload texture dari GPU kalau emang udah ke-load
        if (tilesonMap->tilesetTexture.id != 0)
            UnloadTexture(tilesonMap->tilesetTexture);

        delete tilesonMap;
        tilesonMap = nullptr;
    }
    // bebaskan parsed map dari Tileson
    parsedMap.reset();
}

void TilesonInit(GameState *state)
{
    TilesonLoadMap("world_json/exampleworldmap_2.json");
}

// ngerendering
void TilesonRender(GameState *state)
{
    // kalau map atau texture belum siap, skip
    if (tilesonMap == nullptr || tilesonMap->tilesetTexture.id == 0)
        return;

    BeginMode2D(camera);

    // render semua layer dari bawah ke atas
    for (int l = 0; l < tilesonMap->layerCount; l++)
    {
        for (int y = 0; y < tilesonMap->height; y++)
        {
            for (int x = 0; x < tilesonMap->width; x++)
            {
                int tileId = tilesonMap->tiles[l][(y * tilesonMap->width) + x];
                // tile ID 0 = kosong, skip
                if (tileId == 0)
                    continue;

                int spacing = tilesonMap->tilesetSpacing;
                int firstgid = tilesonMap->tilesetFirstgid;
                int adjustedId = tileId - firstgid; // sesuaiin ID relatif ke tileset
                int tilesetCols = tilesonMap->tilesetCols;

                // hitung posisi potongan tile di image tileset
                int srcX = (adjustedId % tilesetCols) * (TILE_SIZE + spacing);
                int srcY = (adjustedId / tilesetCols) * (TILE_SIZE + spacing);

                Rectangle srcRec = {(float)srcX, (float)srcY, (float)TILE_SIZE, (float)TILE_SIZE};
                // posisi tile di world/layar
                Rectangle dstRec = {(float)(x * TILE_SIZE), (float)(y * TILE_SIZE), (float)TILE_SIZE, (float)TILE_SIZE};

                DrawTexturePro(tilesonMap->tilesetTexture, srcRec, dstRec, (Vector2){0, 0}, 0.0F, WHITE);
            }
        }
    }

    EndMode2D();
}

std::vector<MapObject> TilesonGetObjectsByType(const std::string &type)
{
    std::vector<MapObject> result;
    for (auto &obj : tilesonMap->Objects)
        if (obj.type == type)
            result.push_back(obj);
    return result;
}

MapObject *TilesonGetObjectByName(const std::string &name)
{
    for (auto &obj : tilesonMap->Objects)
        if (obj.name == name)
            return &obj;
    return nullptr;
}