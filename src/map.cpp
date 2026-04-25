/**
 * @file map.cpp
 * @brief Implementasi dari Map System & Tileson Integration Module
 *
 * File ini berisi implementasi sistem map untuk:
 * - Load dan unload map dari file JSON Tiled
 * - Render tile map ke world space
 * - Hitung tile yang visible untuk frustum culling
 * - Handle perpindahan map dan riwayat navigasi
 */

#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/screen.h"
#include "../include/entities.h"
#include "../include/mapLogic.h"
#include "../include/map.h"
#include "../include/tiles.h"
#include "../include/animation.h"
#include "../include/player.h"
#include "../include/mapstack.h"
#include "../include/entities.h"
#include "../include/movement.h"
#include <memory>
#include <string>

/*==============================================================================
 * Global Variables
 *==============================================================================*/

/** Pointer ke data map yang sedang aktif */
TilesonMapData *tilesonMap = nullptr;

/** Hasil parse map dari Tileson */
static std::unique_ptr<tson::Map> parsedMap = nullptr;

/** Camera global untuk rendering world */
Camera2D camera = {0};

/** Jumlah tile yang dirender pada frame terakhir */
int lastTilesRendered = 0;

/** Range tile yang visible pada frame terakhir */
TileRange currentVisibleRange = {0, 0, 0, 0};

/** Stack riwayat perpindahan map */
static MapSystem::MapStack mapHistoryStack;

/** Path map yang sedang aktif */
static std::string currentMapPath = "";

/*==============================================================================
 * Map Loading & Unloading
 *==============================================================================*/

/**
 * @brief Load map dari file JSON Tiled
 *
 * Proses load mencakup:
 * - Parse file map dengan Tileson
 * - Simpan data tile untuk setiap tile layer
 * - Simpan seluruh object dari object layer
 * - Load texture tileset yang dipakai map
 *
 * @param mapPath Path file map yang akan dimuat
 */
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

    // Alokasi array tile untuk setiap layer
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
            std::map<std::tuple<int, int>, tson::Tile *> tileData = layer.getTileData();
            for (const auto &[pos, tile] : tileData)
            {
                int x = std::get<0>(pos);
                int y = std::get<1>(pos);
                if (tile != nullptr && x < tilesonMap->width && y < tilesonMap->height)
                    tilesonMap->tiles[layerIndex][(y * tilesonMap->width) + x] = (int)tile->getGid();
            }
            layerIndex++;
        }
    }

    // Debug log sample tile dari layer terakhir yang terbaca
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
                mapObj.id = obj.getId();
                mapObj.name = obj.getName();
                mapObj.type = obj.getType();
                mapObj.layerName = layer.getName();

                tson::Vector2i pos = obj.getPosition();
                tson::Vector2i size = obj.getSize();
                mapObj.bounds = {(float)pos.x, (float)pos.y, (float)size.x, (float)size.y};

                // Simpan polygon dalam world space jika object memilikinya
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

                // Simpan custom properties dari object
                for (auto &[key, prop] : obj.getProperties().getProperties())
                    mapObj.properties[key] = prop;

                tilesonMap->Objects.push_back(mapObj);
            }
        }
    }

    // Step 5: Load seluruh texture tileset yang dipakai map
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
        info.lastgid = (i + 1 < (int)tilesetList.size())
                           ? tilesetList[i + 1].getFirstgid() - 1
                           : INT_MAX;
        UnloadImage(img);

        tilesonMap->tilesets.push_back(info);
        TraceLog(LOG_INFO, "Tileson: Loaded tileset %s (gid %d-%d)", imagePath.c_str(), info.firstgid, info.lastgid);
    }

    TraceLog(LOG_INFO, "Tileson: Map loaded - %dx%d tiles", tilesonMap->width, tilesonMap->height);
}

/**
 * @brief Bersihkan seluruh data map yang sedang aktif
 *
 * Fungsi ini akan menghapus array tile, object map, texture tileset,
 * dan hasil parse Tileson dari map sebelumnya.
 */
void UnloadMap(void)
{
    if (tilesonMap != nullptr)
    {
        // Hapus data tile tiap layer
        for (int i = 0; i < tilesonMap->layerCount; i++)
            delete[] tilesonMap->tiles[i];
        delete[] tilesonMap->tiles;

        tilesonMap->Objects.clear();

        // Unload texture tileset dari GPU
        for (auto &ts : tilesonMap->tilesets)
            if (ts.texture.id != 0)
                UnloadTexture(ts.texture);
        tilesonMap->tilesets.clear();

        delete tilesonMap;
        tilesonMap = nullptr;
    }

    parsedMap.reset();
}

/**
 * @brief Inisialisasi map pertama saat game dimulai
 *
 * Map awal yang aktif ditentukan langsung di fungsi ini.
 */
void InitMap(void)
{
    // Beberapa pilihan map yang tersedia (sementara di-comment)
    // LoadMap("world_json/exampleworldmap_2.json");
    // LoadMap("world_json/exampleworldmap.json");
    // LoadMap("world_json/outsideLight.json");
    // LoadMap("world_json/testermap.tmj");
    // LoadMap("world_json/cave.json");
    // LoadMap("world_json/inside.json");
    // LoadMap("world_json/light.json");

    // LoadMap("world_json/floorA.json");
    // LoadMap("world_json/floorB.json");
    // LoadMap("world_json/floorC.json");

    // Map yang aktif saat ini
    currentMapPath = "world_json/tutorial.json";
    LoadMap(currentMapPath.c_str());
    BuildMapObjectIndex();
}

/*==============================================================================
 * Map Rendering
 *==============================================================================*/

/**
 * @brief Render seluruh tile map yang sedang terlihat di layar
 *
 * Rendering dilakukan per layer dan hanya untuk tile dalam visible range
 * agar jumlah tile yang digambar tetap efisien.
 */
void RenderMap(void)
{
    // Skip kalau map atau tilesets belum siap
    if (tilesonMap == nullptr || tilesonMap->tilesets.empty())
        return;

    // Ambil range tile yang visible untuk frustum culling
    currentVisibleRange = GetVisibleTileRange();

    lastTilesRendered = 0;

    static bool logged = false;
    if (!logged)
    {
        TraceLog(LOG_INFO, "layerCount=%d", tilesonMap->layerCount);
        logged = true;
    }

    BeginMode2D(camera);

    // Render semua layer, tapi hanya tile yang ada di area visible
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

                // Cari tileset yang sesuai berdasarkan range gid
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

                // Hitung source rectangle pada texture tileset
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

/**
 * @brief Hitung range tile yang sedang terlihat di layar
 *
 * Nilai range dihitung dari viewport camera dalam world space,
 * lalu dikonversi ke index tile dan di-clamp ke batas map.
 *
 * @return Range tile visible yang dipakai oleh RenderMap()
 */
TileRange GetVisibleTileRange(void)
{
    // Pojok kiri-atas dan kanan-bawah layar dalam world space
    Vector2 worldMin = GetScreenToWorld2D({0.0f, 0.0f}, camera);
    Vector2 worldMax = GetScreenToWorld2D({(float)GameScreenWidth, (float)GameScreenHeight}, camera);

    TileRange range;

    // Konversi ke tile index dan tambahkan margin biar gak ada pop-in
    range.minX = (int)floorf(worldMin.x / TILE_SIZE) - 1;
    range.minY = (int)floorf(worldMin.y / TILE_SIZE) - 1;
    range.maxX = (int)ceilf(worldMax.x / TILE_SIZE) + 1;
    range.maxY = (int)ceilf(worldMax.y / TILE_SIZE) + 1;

    // Clamp ke batas map
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
 * @brief Pindah ke map baru dan spawn di pintu atau titik tujuan tertentu
 *
 * Map aktif saat ini akan disimpan ke history sebelum map baru dimuat.
 *
 * @param newMapPath Path file map tujuan
 * @param targetDoorName Nama pintu atau spawn point pada map tujuan
 */
void SwitchMap(const char *newMapPath, const char *targetDoorName)
{
    // Safety check biar gak load path kosong
    if (newMapPath == nullptr || newMapPath[0] == '\0')
    {
        TraceLog(LOG_ERROR, "SwitchMap: newMapPath is null or empty");
        return;
    }

    // Simpan map sekarang ke history sebelum pindah
    if (!currentMapPath.empty())
        mapHistoryStack.Push(currentMapPath, "");

    // Update current map lalu muat map baru
    currentMapPath = newMapPath;

    UnloadMap();
    LoadMap(newMapPath);
    BuildMapObjectIndex();

    // Stop kalau load map gagal
    if (tilesonMap == nullptr)
    {
        TraceLog(LOG_ERROR, "SwitchMap: failed to load map: %s", newMapPath);
        return;
    }

    // Re-init player berdasarkan target spawn di map baru
    PlayerInstance.Init(gState, targetDoorName);

    // Bersihkan entitas map sebelumnya (kecuali player) dan spawn musuh baru
    Entities::Clear();
    Entities::Add(&PlayerInstance);
    SpawnEnemiesFromMap();

    // Set camera ke tengah spawn player
    Vector2 spawnPos = PlayerInstance.GetPosition();
    camera.target = {spawnPos.x + (TILE_SIZE / 2.0F), spawnPos.y + (TILE_SIZE / 2.0F)};
    camera.offset = {(float)(GameScreenWidth / 2), (float)(GameScreenHeight / 2)};
    camera.rotation = 0;
    camera.zoom = 1.0F;

    // Sinkronisasi kamera segera agar tidak ada blink/jump zoom di frame pertama
    Movement::UpdateCamera(PlayerInstance);

    TraceLog(LOG_INFO, "SwitchMap: switched to map: %s via door: %s",
             newMapPath,
             (targetDoorName != nullptr && targetDoorName[0] != '\0') ? targetDoorName : SPAWN_OBJECT_NAME);
}

/**
 * @brief Kembali ke map sebelumnya dari history
 *
 * Fungsi ini memuat ulang map sebelumnya lalu mengatur ulang posisi player dan camera.
 */
void GoBack(void)
{
    if (mapHistoryStack.IsEmpty())
    {
        TraceLog(LOG_WARNING, "GoBack: no map history to go back to");
        return;
    }

    // Ambil map terakhir dari history
    MapSystem::MapHistoryEntry prev = mapHistoryStack.Pop();
    currentMapPath = prev.mapPath;

    // Muat kembali map sebelumnya
    UnloadMap();
    LoadMap(prev.mapPath.c_str());

    if (tilesonMap == nullptr)
    {
        TraceLog(LOG_ERROR, "GoBack: failed to load map: %s", prev.mapPath.c_str());
        return;
    }

    // Init player di spawn point map sebelumnya
    PlayerInstance.Init(gState, prev.doorName.empty() ? SPAWN_OBJECT_NAME : prev.doorName.c_str());

    // Bersihkan entitas map sebelumnya (kecuali player) dan spawn musuh baru
    Entities::Clear();
    Entities::Add(&PlayerInstance);
    SpawnEnemiesFromMap();

    // Set camera ke tengah spawn player
    Vector2 spawnPos = PlayerInstance.GetPosition();
    camera.target = {spawnPos.x + (TILE_SIZE / 2.0F), spawnPos.y + (TILE_SIZE / 2.0F)};
    camera.offset = {(float)(GameScreenWidth / 2), (float)(GameScreenHeight / 2)};
    camera.rotation = 0;
    camera.zoom = 1.0F;

    // Sinkronisasi kamera segera agar tidak ada blink/jump zoom di frame pertama
    Movement::UpdateCamera(PlayerInstance);

    TraceLog(LOG_INFO, "GoBack: returned to map: %s", prev.mapPath.c_str());
}

const char* GetCurrentMapPath(void)
{
    return currentMapPath.c_str();
}
