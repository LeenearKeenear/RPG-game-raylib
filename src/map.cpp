/**
 * @file map.cpp
 * @brief Implementasi dari Map System & Tileson Integration Module
 *
 * Implementasi dari fungsi-fungsi map yang dideklarasikan di map.h
 * Handle loading, rendering, dan management map dari JSON Tiled.
 */

#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/screen.h"
#include "../include/mapLogic.h"
#include "../include/map.h"
#include "../include/animation.h"
#include "../include/player.h"
#include "../include/MapStack.h"
#include <memory>
#include <string>

/*==============================================================================
 * Global Variables
 *==============================================================================*/

/** Global pointer ke data map yang lagi aktif */
TilesonMapData *tilesonMap = nullptr;

/** Hasil parse Tileson - unique_ptr buat auto cleanup */
static std::unique_ptr<tson::Map> parsedMap = nullptr;

/** Camera2D global untuk rendering world */
Camera2D camera = {0};

/** Jumlah tile yang dirender di frame terakhir (buat debug) */
int lastTilesRendered = 0;

/** Range tile visible di frame terakhir (buat debug) */
TileRange currentVisibleRange = {0, 0, 0, 0};

/** Stack buat nyimpen riwayat perpindahan map (fitur go back) */
static MapSystem::MapStack mapHistoryStack;

/** Path file map yang sedang aktif */
static std::string currentMapPath = "";

/*==============================================================================
 * Map Loading & Unloading
 *==============================================================================*/

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
    // Step 1: Parse file JSON dengan Tileson
    tson::Tileson t;
    parsedMap = t.parse(mapPath);

    if (parsedMap->getStatus() != tson::ParseStatus::OK)
    {
        TraceLog(LOG_ERROR, "Tileson: Failed to parse map: %s", parsedMap->getStatusMessage().c_str());
        return;
    }

    // Step 2: Alokasi struktur data map
    tilesonMap = new TilesonMapData();
    tson::Vector2i mapSize = parsedMap->getSize();
    tilesonMap->width = mapSize.x;
    tilesonMap->height = mapSize.y;

    // Hitung jumlah TileLayer dulu sebelum alokasi array
    int layerCount = 0;
    for (auto &layer : parsedMap->getLayers())
        if (layer.getType() == tson::LayerType::TileLayer)
            layerCount++;

    // Alokasi array 2D — satu slot per layer
    tilesonMap->layerCount = layerCount;
    tilesonMap->tiles = new int *[layerCount];
    for (int i = 0; i < layerCount; i++)
        tilesonMap->tiles[i] = new int[tilesonMap->width * tilesonMap->height]();

    // Step 3: Isi data tile tiap layer
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

    // Debug log buat ngecek sample tile (layer pertama)
    TraceLog(LOG_INFO, "Layer %d: sample tileId[0]=%d tileId[1]=%d",
             layerIndex - 1,
             tilesonMap->tiles[layerIndex - 1][0],
             tilesonMap->tiles[layerIndex - 1][1]);

    // Step 4: Baca semua object dari ObjectGroup layer
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

                // Kalo object punya polygon, simpan semua titiknya dalam world space
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

                // Ambil semua custom properties dari object
                for (auto &[key, prop] : obj.getProperties().getProperties())
                    mapObj.properties[key] = prop;

                tilesonMap->Objects.push_back(mapObj);
            }
        }
    }

    // Step 5: Load texture tileset — nama file diambil dari JSON, di-prefix texture/
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
        // Hapus tiap array tile per layer dulu sebelum hapus array utamanya
        for (int i = 0; i < tilesonMap->layerCount; i++)
            delete[] tilesonMap->tiles[i];
        delete[] tilesonMap->tiles;

        tilesonMap->Objects.clear();

        // Unload texture dari GPU
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
//
// ================================================================
void InitMap(void)
{
    // Beberapa pilihan map yang tersedia (sementara di-comment)
    // LoadMap("world_json/exampleworldmap_2.json");
    // LoadMap("world_json/exampleworldmap.json");
    // LoadMap("world_json/outsideLight.json");
    // LoadMap("world_json/testermap.tmj");
    // LoadMap("world_json/cave.json");
    // LoadMap("world_json/inside.json");

    // Map yang aktif saat ini
    LoadMap("world_json/light.json");
    BuildMapObjectIndex();

}

/*==============================================================================
 * Map Rendering
 *==============================================================================*/

// ================================================================
// RenderMap()
// Render semua tile layer dari bawah ke atas dalam world space.
// Dipanggil dari DrawRenderTexture() sebelum RenderEntities().
// ================================================================
void RenderMap(void)
{
    // Skip kalau map atau tilesets belum siap
    if (tilesonMap == nullptr || tilesonMap->tilesets.empty())
        return;

    // Dapatkan range tile yang visible dari frustum culling di player
    currentVisibleRange = GetVisibleTileRange();

    lastTilesRendered = 0; // reset counter per frame

    static bool logged = false;
    if (!logged)
    {
        TraceLog(LOG_INFO, "layerCount=%d", tilesonMap->layerCount);
        logged = true;
    }

    BeginMode2D(camera);

    // Loop semua layer, lalu tile yang visible aja
    for (int l = 0; l < tilesonMap->layerCount; l++)
    {
        for (int y = currentVisibleRange.minY; y < currentVisibleRange.maxY; y++)
        {
            for (int x = currentVisibleRange.minX; x < currentVisibleRange.maxX; x++)
            {
                lastTilesRendered++;
                int tileId = tilesonMap->tiles[l][(y * tilesonMap->width) + x];
                if (tileId == 0)
                    continue; // tile kosong, skip

                // Cari tileset yang sesuai berdasarkan tileId (range firstgid - lastgid)
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
                    continue; // tileset gak ketemu, skip

                // Hitung posisi source di spritesheet
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
// GetVisibleTileRange()
// Inti logic frustum culling — hitung range tile yang visible
// di layar berdasarkan camera viewport saat ini.
//
// Cara kerja:
// 1. Konversi pojok kiri-atas layar ke world space → worldMin
// 2. Konversi pojok kanan-bawah layar ke world space → worldMax
// 3. Bagi koordinat world dengan TILE_SIZE → dapat index tile
// 4. Tambah margin 1 tile di tiap sisi biar gak ada pop-in di tepi
// 5. Clamp ke batas map yang valid (0 .. width/height)
//
// Dipanggil oleh RenderMap() setiap frame.
// ================================================================
TileRange GetVisibleTileRange(void)
{
    // Pojok kiri-atas dan kanan-bawah layar dalam world space
    Vector2 worldMin = GetScreenToWorld2D({0.0f, 0.0f}, camera);
    Vector2 worldMax = GetScreenToWorld2D({(float)GameScreenWidth, (float)GameScreenHeight}, camera);

    TileRange range;

    // Konversi ke tile index + margin 1 tile biar gak ada pop-in di tepi
    range.minX = (int)floorf(worldMin.x / TILE_SIZE) - 1;
    range.minY = (int)floorf(worldMin.y / TILE_SIZE) - 1;
    range.maxX = (int)ceilf(worldMax.x / TILE_SIZE) + 1;
    range.maxY = (int)ceilf(worldMax.y / TILE_SIZE) + 1;

    // Clamp ke batas map yang valid (0 .. width/height)
    if (range.minX < 0)
        range.minX = 0;
    if (range.minY < 0)
        range.minY = 0;
    if (range.maxX > tilesonMap->width)
        range.maxX = tilesonMap->width;
    if (range.maxY > tilesonMap->height)
        range.maxY = tilesonMap->height;

    return range;
}

/*==============================================================================
 * Map Switching & Navigation
 *==============================================================================*/

/**
 * @brief Pindah ke map baru di posisi pintu tertentu
 * @param newMapPath Path file map tujuan (.tmj)
 * @param targetDoorName Nama object pintu di map baru (spawn point)
 * @note Otomatis push map lama ke stack, unload, load baru, dan teleport player
 */
void SwitchMap(const char *newMapPath, const char *targetDoorName)
{
    // Safety check biar gak load path kosong
    if (newMapPath == nullptr || newMapPath[0] == '\0')
    {
        TraceLog(LOG_ERROR, "SwitchMap: newMapPath is null or empty");
        return;
    }

    // Push map sekarang ke stack sebelum pindah
    // Skip kalau currentMapPath kosong (berarti ini load pertama kali)
    if (!currentMapPath.empty())
        mapHistoryStack.Push(currentMapPath, "");

    // Update currentMapPath ke map baru
    currentMapPath = newMapPath;

    // Unload map lama dulu kalau ada
    UnloadMap();

    // Load map baru
    LoadMap(newMapPath);
    BuildMapObjectIndex();

    // Kalau gagal load, jangan lanjut init player/camera
    if (tilesonMap == nullptr)
    {
        TraceLog(LOG_ERROR, "SwitchMap: failed to load map: %s", newMapPath);
        return;
    }

    // Re-init player berdasarkan target door di map baru
    PlayerInstance.Init(gState, targetDoorName);

    // Set camera ke tengah spawn player
    Vector2 spawnPos = PlayerInstance.GetPosition();
    camera.target = {spawnPos.x + (TILE_SIZE / 2.0F), spawnPos.y + (TILE_SIZE / 2.0F)};
    camera.offset = {(float)(GameScreenWidth / 2), (float)(GameScreenHeight / 2)};
    camera.rotation = 0;
    camera.zoom = 1.0F;

    TraceLog(LOG_INFO, "SwitchMap: switched to map: %s via door: %s",
             newMapPath,
             (targetDoorName != nullptr && targetDoorName[0] != '\0') ? targetDoorName : SPAWN_OBJECT_NAME);
}

/**
 * @brief Kembali ke map sebelumnya (fitur go back)
 * @note Pop dari stack, unload map sekarang, load map sebelumnya, teleport player
 */
void GoBack(void)
{
    if (mapHistoryStack.IsEmpty())
    {
        TraceLog(LOG_WARNING, "GoBack: no map history to go back to");
        return;
    }

    // Ambil history teratas dan pop dari stack
    MapSystem::MapHistoryEntry prev = mapHistoryStack.Pop();
    currentMapPath = prev.mapPath;

    // Unload map sekarang, load map sebelumnya
    UnloadMap();
    LoadMap(prev.mapPath.c_str());

    if (tilesonMap == nullptr)
    {
        TraceLog(LOG_ERROR, "GoBack: failed to load map: %s", prev.mapPath.c_str());
        return;
    }

    // Init player di spawn point map sebelumnya
    PlayerInstance.Init(gState, prev.doorName.empty() ? SPAWN_OBJECT_NAME : prev.doorName.c_str());

    // Set camera ke tengah spawn player
    Vector2 spawnPos = PlayerInstance.GetPosition();
    camera.target = {spawnPos.x + (TILE_SIZE / 2.0F), spawnPos.y + (TILE_SIZE / 2.0F)};
    camera.offset = {(float)(GameScreenWidth / 2), (float)(GameScreenHeight / 2)};
    camera.rotation = 0;
    camera.zoom = 1.0F;

    TraceLog(LOG_INFO, "GoBack: returned to map: %s", prev.mapPath.c_str());
}