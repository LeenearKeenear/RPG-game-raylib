#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/screen.h"
#include "../include/map.h"
#include "../include/animation.h"
#include "../include/player.h"
#include <memory>
#include <string>

// ================================================================
// Global Variables
// ================================================================

TilesonMapData *tilesonMap = nullptr;
static std::unique_ptr<tson::Map> parsedMap = nullptr;
Camera2D camera = {0};

int lastTilesRendered = 0;
TileRange currentVisibleRange = {0, 0, 0, 0};

// ================================================================
// LoadMap()
// Parse file JSON Tiled dan load semua data map ke TilesonMapData.
//
// Cara kerja:
// 1. Parse JSON pake Tileson
// 2. Baca semua TileLayer → simpen ke tilesonMap->tiles
// 3. Baca semua ObjectGroup → simpen ke tilesonMap->Objects
// 4. Load texture tileset dari folder texture/
// ================================================================
void LoadMap(const char *mapPath)
{
    tson::Tileson t;
    parsedMap = t.parse(mapPath);

    if (parsedMap->getStatus() != tson::ParseStatus::OK)
    {
        TraceLog(LOG_ERROR, "Tileson: Failed to parse map: %s", parsedMap->getStatusMessage().c_str());
        return;
    }

    tilesonMap = new TilesonMapData();
    tson::Vector2i mapSize = parsedMap->getSize();
    tilesonMap->width = mapSize.x;
    tilesonMap->height = mapSize.y;

    // hitung jumlah TileLayer dulu sebelum alokasi
    int layerCount = 0;
    for (auto &layer : parsedMap->getLayers())
        if (layer.getType() == tson::LayerType::TileLayer)
            layerCount++;

    // alokasi array 2D — satu slot per layer
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
            // tileData: key = (x,y), value = pointer ke Tile
            std::map<std::tuple<int, int>, tson::Tile *> tileData = layer.getTileData();
            for (const auto &[pos, tile] : tileData)
            {
                int x = std::get<0>(pos);
                int y = std::get<1>(pos);
                if (tile != nullptr && x < tilesonMap->width && y < tilesonMap->height)
                    // konversi posisi 2D ke index 1D — pakai getGid() bukan getId()
                    tilesonMap->tiles[layerIndex][(y * tilesonMap->width) + x] = (int)tile->getGid();
            }
            layerIndex++;
        }
    }

    // setelah layerIndex++;
    TraceLog(LOG_INFO, "Layer %d: sample tileId[0]=%d tileId[1]=%d",
             layerIndex - 1,
             tilesonMap->tiles[layerIndex - 1][0],
             tilesonMap->tiles[layerIndex - 1][1]);

    // baca semua object dari ObjectGroup layer
    for (auto &layer : parsedMap->getLayers())
    {
        if (layer.getType() == tson::LayerType::ObjectGroup)
        {
            for (auto &obj : layer.getObjects())
            {
                MapObject mapObj;
                mapObj.name = obj.getName();
                mapObj.type = obj.getType();
                mapObj.layerName = layer.getName();

                tson::Vector2i pos = obj.getPosition();
                tson::Vector2i size = obj.getSize();
                mapObj.bounds = {(float)pos.x, (float)pos.y, (float)size.x, (float)size.y};

                // kalau object punya polygon, simpan semua titiknya dalam world space
                const std::vector<tson::Vector2i> &polygon = obj.getPolygons();
                if (!polygon.empty())
                {
                    mapObj.hasPolygon = true;

                    for (const auto &point : polygon)
                    {
                        mapObj.polygonPoints.push_back({(float)pos.x + (float)point.x,
                                                        (float)pos.y + (float)point.y});
                    }
                }

                // ambil semua custom properties dari object
                for (auto &[key, prop] : obj.getProperties().getProperties())
                    mapObj.properties[key] = prop;

                tilesonMap->Objects.push_back(mapObj);
            }
        }
    }

    // load texture tileset — nama file diambil dari JSON, di-prefix texture/
    auto &tilesetList = parsedMap->getTilesets();
    for (int i = 0; i < (int)tilesetList.size(); i++)
    {
        tson::Tileset *tileset = &tilesetList[i];
        std::string imagePath = "texture/" + tileset->getImagePath().filename().u8string();
        TraceLog(LOG_INFO, "Tileson: Loading tileset: %s", imagePath.c_str());

        Image img = LoadImage(imagePath.c_str());
        TilesetInfo info;
        info.texture = LoadTextureFromImage(img);
        info.cols = tileset->getColumns();
        info.spacing = tileset->getSpacing();
        info.firstgid = tileset->getFirstgid();
        // lastgid = firstgid tileset berikutnya - 1, tileset terakhir pakai INT_MAX
        info.lastgid = (i + 1 < (int)tilesetList.size())
                           ? tilesetList[i + 1].getFirstgid() - 1
                           : INT_MAX;
        UnloadImage(img);

        tilesonMap->tilesets.push_back(info);
        TraceLog(LOG_INFO, "Tileson: Loaded tileset %s (gid %d-%d)", imagePath.c_str(), info.firstgid, info.lastgid);
    }

    TraceLog(LOG_INFO, "Tileson: Map loaded - %dx%d tiles", tilesonMap->width, tilesonMap->height);
}

// ================================================================
// UnloadMap()
// Bersihin semua memory yang dialokasiin pas LoadMap().
// Harus dipanggil sebelum load map baru atau pas game shutdown.
// ================================================================
void UnloadMap(void)
{
    if (tilesonMap != nullptr)
    {
        // hapus tiap array tile per layer dulu sebelum hapus array utamanya
        for (int i = 0; i < tilesonMap->layerCount; i++)
            delete[] tilesonMap->tiles[i];
        delete[] tilesonMap->tiles;

        tilesonMap->Objects.clear();

        // unload texture dari GPU
        for (auto &ts : tilesonMap->tilesets)
            if (ts.texture.id != 0)
                UnloadTexture(ts.texture);
        tilesonMap->tilesets.clear();

        delete tilesonMap;
        tilesonMap = nullptr;
    }

    parsedMap.reset();
}

// ================================================================
// InitMap()
// Entry point load map — path map di-hardcode di sini.
// TODO MULTI-MAP: nanti path-nya bisa dioper sebagai parameter
// atau dibaca dari save data buat support pindah antar map.
// ================================================================
void InitMap(void)
{
    // LoadMap("world_json/exampleworldmap_2.json");
    // LoadMap("world_json/exampleworldmap.json");
    
    // LoadMap("world_json/outsideLight.json");
    // LoadMap("world_json/testermap.tmj");


    // LoadMap("world_json/cave.json");
    LoadMap("world_json/inside.json");
    // LoadMap("world_json/light.json");
}

// ================================================================
// RenderMap()
// Render semua tile layer dari bawah ke atas dalam world space.
// Dipanggil dari DrawRenderTexture() sebelum RenderEntities().
// ================================================================
void RenderMap(void)
{
    // skip kalau map atau tilesets belum siap
    if (tilesonMap == nullptr || tilesonMap->tilesets.empty())
        return;

    // dapatkan range tile yang visible dari inti logic frustum culling di player
    currentVisibleRange = PlayerInstance.GetVisibleTileRange();

    lastTilesRendered = 0; // reset counter per frame

    static bool logged = false;
    if (!logged)
    {
        TraceLog(LOG_INFO, "layerCount=%d", tilesonMap->layerCount);
        logged = true;
    }

    BeginMode2D(camera);

    for (int l = 0; l < tilesonMap->layerCount; l++)
    {
        for (int y = currentVisibleRange.minY; y < currentVisibleRange.maxY; y++)
        {
            for (int x = currentVisibleRange.minX; x < currentVisibleRange.maxX; x++)
            {
                lastTilesRendered++;
                int tileId = tilesonMap->tiles[l][(y * tilesonMap->width) + x];
                if (tileId == 0)
                    continue;

                // cari tileset yang sesuai berdasarkan tileId
                TilesetInfo *ts = nullptr;
                for (auto &t : tilesonMap->tilesets)
                    if (tileId >= t.firstgid && tileId <= t.lastgid)
                    {
                        ts = &t;
                        break;
                    }

                static bool logged = false;
                if (!logged)
                {
                    TraceLog(LOG_INFO, "FIRST: tileId=%d", tileId);
                    logged = true;
                }

                if (ts == nullptr)
                    continue;

                // taruh di sini
                if (!logged)
                {
                    TraceLog(LOG_INFO, "FIRST: tileId=%d firstgid=%d lastgid=%d adjustedId=%d",
                             tileId, ts->firstgid, ts->lastgid, tileId - ts->firstgid);
                    logged = true;
                }

                int adjustedId = tileId - ts->firstgid;
                int srcX = (adjustedId % ts->cols) * (TILE_SIZE + ts->spacing);
                int srcY = (adjustedId / ts->cols) * (TILE_SIZE + ts->spacing);

                Rectangle srcRec = {(float)srcX, (float)srcY, (float)TILE_SIZE, (float)TILE_SIZE};
                Rectangle dstRec = {(float)(x * TILE_SIZE), (float)(y * TILE_SIZE), (float)TILE_SIZE, (float)TILE_SIZE};

                DrawTexturePro(ts->texture, srcRec, dstRec, (Vector2){0, 0}, 0.0F, WHITE);
            }
        }
    }

    EndMode2D();
}

// ================================================================
// TilesonGetObjectsByLayerName()
// Ambil semua object dari tilesonMap->Objects yang berasal dari
// object layer dengan nama yang cocok.
//
// Dipake buat sistem yang identitasnya berbasis layer name,
// misalnya collision dan custom world boundary.
// ================================================================
std::vector<MapObject> TilesonGetObjectsByLayerName(const std::string &layerName)
{
    std::vector<MapObject> result;
    for (auto &obj : tilesonMap->Objects)
        if (obj.layerName == layerName)
            result.push_back(obj);
    return result;
}

// ================================================================
// TilesonGetObjectsByType()
// Ambil semua object dari tilesonMap->Objects yang type-nya cocok.
// Dipake buat load collision rectangles pas Player::Init().
// ================================================================
std::vector<MapObject> TilesonGetObjectsByType(const std::string &type)
{
    std::vector<MapObject> result;
    for (auto &obj : tilesonMap->Objects)
        if (obj.type == type)
            result.push_back(obj);
    return result;
}

// ================================================================
// TilesonGetObjectByName()
// Cari object pertama yang namanya cocok dari tilesonMap->Objects.
// Dipake buat nyari spawn point pas Player::Init().
// Return nullptr kalau gak ketemu.
// ================================================================
MapObject *TilesonGetObjectByName(const std::string &name)
{
    for (auto &obj : tilesonMap->Objects)
        if (obj.name == name)
            return &obj;
    return nullptr;
}