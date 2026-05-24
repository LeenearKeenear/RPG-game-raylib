#include "worldgenenartion.h"
#include "mapLogic.h"
#include <filesystem>
#include <random>
#include <vector>

extern TilesonMapData *tilesonMap;
static std::mt19937 wgRng(std::random_device{}());
uint64_t worldSeed = 12345231311; // placeholder, nanti diganti random_device

uint64_t SplitMix64(uint64_t x)
{
    x += 0x9e3779b97f4a7c15;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
    x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
    return x ^ (x >> 31);
}

int PickCorridorIndex(int tileX, int tileY, int exitTypeHash, int poolSize)
{
    static const uint64_t EXIT_SALT[4] = {
        0x9e3779b97f4a7c15,
        0xbf58476d1ce4e5b9,
        0x94d049bb133111eb,
        0x1234567890abcdef
    };

    uint64_t posHash = SplitMix64((uint64_t)tileX * WG_CANVAS_TILES + (uint64_t)tileY);
    uint64_t combined = worldSeed + posHash + EXIT_SALT[exitTypeHash - 1];
    return (int)(SplitMix64(combined) % (uint64_t)poolSize);
}

TilesonMapData *GetRandomCorridor(WeightedPool &pool, int tileX, int tileY, int exitTypeHash)
{
    if (pool.prefabs.empty())
        return nullptr;
    int idx = PickCorridorIndex(tileX, tileY, exitTypeHash, (int)pool.prefabs.size());
    return pool.prefabs[idx];
}

std::vector<MapObject> GetWorldGenSlots()
{
    const auto &slots = TilesonGetObjectsByType(SLOT_WORLDGEN_LAYER_NAME);
    TraceLog(LOG_INFO, "WorldGen: slots found = %d", (int)slots.size());
    std::vector<MapObject> result;
    for (auto *ptr : slots)
        result.push_back(*ptr);
    return result;
}

std::vector<std::vector<int>> ReshapeTo2D(const std::vector<int> &flatData, int width, int height)
{
    std::vector<std::vector<int>> grid(height, std::vector<int>(width));
    for (int i = 0; i < (int)flatData.size(); i++)
    {
        grid[i / width][i % width] = flatData[i];
    }
    return grid;
}

std::vector<int> FlattenTo1D(const std::vector<std::vector<int>> &grid)
{
    std::vector<int> flat;
    for (const auto &row : grid)
    {
        for (int val : row)
        {
            flat.push_back(val);
        }
    }
    return flat;
}

std::vector<std::vector<int>> Rotate90CW(const std::vector<std::vector<int>> &grid)
{
    int rows = grid.size();
    int cols = grid[0].size();
    std::vector<std::vector<int>> result(cols, std::vector<int>(rows));
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            result[c][rows - 1 - r] = grid[r][c];
        }
    }
    return result;
}

std::vector<std::vector<int>> RotateGrid(const std::vector<std::vector<int>> &grid, int degrees)
{
    // degrees: 0, 90, 180, 270
    int times = (degrees / 90) % 4;
    std::vector<std::vector<int>> result = grid;
    for (int i = 0; i < times; i++)
    {
        result = Rotate90CW(result);
    }
    return result;
}

Vector2 RotatePoint(Vector2 point, Vector2 center, int degrees)
{
    float x = point.x - center.x;
    float y = point.y - center.y;

    for (int i = 0; i < (degrees / 90) % 4; i++)
    {
        float temp = x;
        x = -y;
        y = temp;
    }

    return {x + center.x, y + center.y};
}

void PreFabLoadMap(const char *mapPath, TilesonMapData *target)
{
    TraceLog(LOG_INFO, "PreFabLoadMap: loading %s", mapPath);
    tson::Tileson t;
    auto parsed = t.parse(mapPath);

    if (parsed->getStatus() != tson::ParseStatus::OK)
    {
        TraceLog(LOG_ERROR, "Tileson: Failed to parse map: %s", parsed->getStatusMessage().c_str());
        return;
    }

    tson::Vector2i mapSize = parsed->getSize();
    target->width = mapSize.x;
    target->height = mapSize.y;

    int layerCount = 0;
    for (auto &layer : parsed->getLayers())
        if (layer.getType() == tson::LayerType::TileLayer)
            layerCount++;

    target->layerCount = layerCount;
    target->tiles = new int *[layerCount];
    for (int i = 0; i < layerCount; i++)
        target->tiles[i] = new int[target->width * target->height]();

    int layerIndex = 0;
    for (auto &layer : parsed->getLayers())
    {
        if (layer.getType() == tson::LayerType::TileLayer)
        {
            std::map<std::tuple<int, int>, tson::Tile *> tileData = layer.getTileData();
            for (const auto &[pos, tile] : tileData)
            {
                int x = std::get<0>(pos);
                int y = std::get<1>(pos);
                if (tile != nullptr && x < target->width && y < target->height)
                    target->tiles[layerIndex][y * target->width + x] = (int)tile->getGid();
            }
            layerIndex++;
        }
    }

    for (auto &layer : parsed->getLayers())
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

                const std::vector<tson::Vector2i> &polygon = obj.getPolygons();
                if (!polygon.empty())
                {
                    mapObj.hasPolygon = true;
                    for (const auto &point : polygon)
                        mapObj.polygonPoints.push_back({(float)pos.x + (float)point.x,
                                                        (float)pos.y + (float)point.y});
                }

                for (auto &[key, prop] : obj.getProperties().getProperties())
                    mapObj.properties[key] = prop;

                target->Objects.push_back(mapObj);
            }
        }
    }

    // Step 5: Load seluruh texture tileset yang dipakai map
    auto &tilesetList = parsed->getTilesets();
    for (int i = 0; i < (int)tilesetList.size(); i++)
    {
        tson::Tileset *tileset = &tilesetList[i];
        std::string imagePath = "assets/textures/" + tileset->getImagePath().filename().string();
        TraceLog(LOG_INFO, "Tileson: Loading tileset: %s", imagePath.c_str());

        TilesetInfo info;

        // Cek apakah texture tile utama sudah dimuat, reuse jika ada
        if (textures[TILESET_MAP_1].id != 0 && imagePath == "assets/textures/tiles.png")
        {
            info.texture = textures[TILESET_MAP_1];
            TraceLog(LOG_INFO, "Tileson: Reusing cached texture for tiles.png");
        }
        else if (textures[TILESET_MAP_2].id != 0 && imagePath == "assets/textures/test.png")
        {
            info.texture = textures[TILESET_MAP_2];
            TraceLog(LOG_INFO, "Tileson: Reusing cached texture for test.png");
        }
        else if (textures[TILESET_PROPS].id != 0 && imagePath == "assets/textures/props.png")
        {
            info.texture = textures[TILESET_PROPS];
            TraceLog(LOG_INFO, "Tileson: Reusing cached texture for props.png");
        }
        else if (textures[TILESET_ITEMS].id != 0 && imagePath == "assets/textures/items.png")
        {
            info.texture = textures[TILESET_ITEMS];
            TraceLog(LOG_INFO, "Tileson: Reusing cached texture for items.png");
        }
        else
        {
            Image img = LoadImage(imagePath.c_str());
            info.texture = LoadTextureFromImage(img);
            UnloadImage(img);
        }

        info.cols = tileset->getColumns();
        info.spacing = tileset->getSpacing();
        info.firstgid = tileset->getFirstgid();
        info.lastgid = (i + 1 < (int)tilesetList.size())
                           ? tilesetList[i + 1].getFirstgid() - 1
                           : INT_MAX;

        if (target->tilesets.empty())
            target->tilesets.push_back({});
        target->tilesets[0].push_back(info);
        target->layerTilesetGroup.assign(target->layerCount, 0);
        TraceLog(LOG_INFO, "Tileson: Loaded tileset %s (gid %d-%d)", imagePath.c_str(), info.firstgid, info.lastgid);
    }

    TraceLog(LOG_INFO, "PreFabLoadMap: done %s", mapPath);
}

void UnloadPrefab(TilesonMapData *prefab)
{
    if (!prefab)
        return;

    if (prefab->tiles)
    {
        for (int i = 0; i < prefab->layerCount; i++)
        {
            delete[] prefab->tiles[i];
        }
        delete[] prefab->tiles;
    }
    delete prefab;
}

void ExpandCanvasLayers(int extraLayers)
{
    int newLayerCount = tilesonMap->layerCount + extraLayers;

    int **newTiles = new int *[newLayerCount];

    for (int i = 0; i < tilesonMap->layerCount; i++)
        newTiles[i] = tilesonMap->tiles[i];

    for (int i = tilesonMap->layerCount; i < newLayerCount; i++)
        newTiles[i] = new int[tilesonMap->width * tilesonMap->height]();

    delete[] tilesonMap->tiles;
    tilesonMap->tiles = newTiles;
    tilesonMap->layerCount = newLayerCount;

    tilesonMap->layerTilesetGroup.resize(newLayerCount, 0);
}

void StampMap(TilesonMapData *source, int offsetX, int offsetY, int targetLayerOffset)
{
    // cek apakah tileset sudah ada
    int newGroupIdx = -1;
    for (int i = 0; i < (int)tilesonMap->tilesets.size(); i++)
    {
        if (tilesonMap->tilesets[i][0].texture.id == source->tilesets[0][0].texture.id)
        {
            newGroupIdx = i;
            break;
        }
    }
    // append tileset group dari prefab
    if (newGroupIdx == -1)
    {
        newGroupIdx = (int)tilesonMap->tilesets.size();
        tilesonMap->tilesets.push_back(source->tilesets[0]);
    }

    // stamp tile, GID gak perlu di-shift
    for (int l = 0; l < source->layerCount; l++)
    {
        int dstLayer = targetLayerOffset + l;
        if (dstLayer >= tilesonMap->layerCount)
            break;

        // assign layer ke group baru
        tilesonMap->layerTilesetGroup[dstLayer] = newGroupIdx;

        for (int y = 0; y < source->height; y++)
        {
            for (int x = 0; x < source->width; x++)
            {
                int srcTile = source->tiles[l][y * source->width + x];
                if (srcTile == 0)
                    continue;

                int cx = offsetX + x;
                int cy = offsetY + y;
                if (cx >= tilesonMap->width || cy >= tilesonMap->height)
                    continue;

                tilesonMap->tiles[dstLayer][cy * tilesonMap->width + cx] = srcTile;
            }
        }
    }

    // stamp objects
    for (auto &obj : source->Objects)
    {
        MapObject stamped = obj;
        stamped.bounds.x += offsetX * WG_TILE_SIZE;
        stamped.bounds.y += offsetY * WG_TILE_SIZE;
        for (auto &p : stamped.polygonPoints)
        {
            p.x += offsetX * WG_TILE_SIZE;
            p.y += offsetY * WG_TILE_SIZE;
        }
        tilesonMap->Objects.push_back(stamped);
    }
}

void StampPrefabToSlot(TilesonMapData *prefab, MapObject *slot)
{
    int slotTileX = (int)(slot->bounds.x / WG_TILE_SIZE);
    int slotTileY = (int)(slot->bounds.y / WG_TILE_SIZE);
    int slotTileW = (int)(slot->bounds.width / WG_TILE_SIZE);
    int slotTileH = (int)(slot->bounds.height / WG_TILE_SIZE);

    int offsetX = slotTileX + (slotTileW - prefab->width) / 2;
    int offsetY = slotTileY + (slotTileH - prefab->height) / 2;

    if (tilesonMap->layerCount < WG_PREFAB_LAYER_START + prefab->layerCount)
        ExpandCanvasLayers((WG_PREFAB_LAYER_START + prefab->layerCount) - tilesonMap->layerCount);

    StampMap(prefab, offsetX, offsetY, 2);
}

TilesonMapData *RotateTileLayers(TilesonMapData *source, int degrees)
{
    TilesonMapData *result = new TilesonMapData();
    result->width = source->width;
    result->height = source->height;
    result->layerCount = source->layerCount;
    result->tilesets = source->tilesets;
    result->layerTilesetGroup = source->layerTilesetGroup;

    result->tiles = new int *[result->layerCount];
    for (int i = 0; i < result->layerCount; i++)
    {
        result->tiles[i] = new int[result->width * result->height]();

        std::vector<int> flat(source->tiles[i], source->tiles[i] + source->width * source->height);
        auto grid2D = ReshapeTo2D(flat, source->width, source->height);
        auto rotated = RotateGrid(grid2D, degrees);
        auto flatResult = FlattenTo1D(rotated);

        for (int j = 0; j < (int)flatResult.size(); j++)
            result->tiles[i][j] = flatResult[j];
    }

    return result;
}

void RotateObjectLayer(TilesonMapData *source, TilesonMapData *result, int degrees, int tileSize, bool obstacleOnly)
{
    Vector2 center = {(source->width * tileSize) / 2.0f, (source->height * tileSize) / 2.0f};

    for (auto &obj : source->Objects)
    {
        bool isObstacle = obj.layerName == COLLISION_LAYER_NAME;
        bool isExit = obj.layerName == EXIT_LAYER_NAME;
        if (obstacleOnly && !isObstacle && !isExit)
            continue;
        if (!obstacleOnly && (isObstacle || isExit))
            continue;

        MapObject rotated = obj;

        Vector2 topLeft = RotatePoint({obj.bounds.x, obj.bounds.y}, center, degrees);
        Vector2 topRight = RotatePoint({obj.bounds.x + obj.bounds.width, obj.bounds.y}, center, degrees);
        Vector2 bottomLeft = RotatePoint({obj.bounds.x, obj.bounds.y + obj.bounds.height}, center, degrees);
        Vector2 bottomRight = RotatePoint({obj.bounds.x + obj.bounds.width, obj.bounds.y + obj.bounds.height}, center, degrees);

        float minX = std::min({topLeft.x, topRight.x, bottomLeft.x, bottomRight.x});
        float minY = std::min({topLeft.y, topRight.y, bottomLeft.y, bottomRight.y});
        float maxX = std::max({topLeft.x, topRight.x, bottomLeft.x, bottomRight.x});
        float maxY = std::max({topLeft.y, topRight.y, bottomLeft.y, bottomRight.y});

        rotated.bounds = {minX, minY, maxX - minX, maxY - minY};

        if (obj.hasPolygon)
        {
            rotated.polygonPoints.clear();
            for (auto &point : obj.polygonPoints)
            {
                rotated.polygonPoints.push_back(RotatePoint(point, center, degrees));
            }
        }

        if (isExit)
        {
            static const std::map<std::string, std::string> exitRemap90CW = {
                {"exit_north", "exit_east"},
                {"exit_east", "exit_south"},
                {"exit_south", "exit_west"},
                {"exit_west", "exit_north"}};

            int times = (degrees / 90) % 4;
            std::string remappedName = rotated.name;
            for (int i = 0; i < times; i++)
            {
                auto it = exitRemap90CW.find(remappedName);
                if (it != exitRemap90CW.end())
                    remappedName = it->second;
            }
            rotated.name = remappedName;
            rotated.type = remappedName;
        }

        result->Objects.push_back(rotated);
    }
}

TilesonMapData *RotatePrefab(TilesonMapData *source, int tileDegrees, int objectDegrees)
{
    TilesonMapData *result = RotateTileLayers(source, tileDegrees);
    RotateObjectLayer(source, result, tileDegrees, FRAME_SIZE, true);
    RotateObjectLayer(source, result, objectDegrees, FRAME_SIZE, false);
    return result;
}

CorridorPool corridorPool;

void LoadCorridorPool(const char *basePath)
{
    if (corridorPool.loaded)
        return;

    namespace fs = std::filesystem;

    // Scan corridor_v
    std::string vPath = std::string(basePath) + "/corridor_v";
    int vCount = 0;
    for (auto &entry : fs::directory_iterator(vPath))
    {
        if (entry.path().extension() != ".json")
            continue;
        TilesonMapData *data = new TilesonMapData();
        PreFabLoadMap(entry.path().string().c_str(), data);
        corridorPool.vertical.prefabs.push_back(data);
        vCount++;
    }
    // Generate 180° rotated variants
    for (int i = 0; i < vCount; i++)
    {
        TilesonMapData *rot = RotatePrefab(corridorPool.vertical.prefabs[i], 180, 180);
        corridorPool.vertical.prefabs.push_back(rot);
    }

    // Scan corridor_h
    std::string hPath = std::string(basePath) + "/corridor_h";
    int hCount = 0;
    for (auto &entry : fs::directory_iterator(hPath))
    {
        if (entry.path().extension() != ".json")
            continue;
        TilesonMapData *data = new TilesonMapData();
        PreFabLoadMap(entry.path().string().c_str(), data);
        corridorPool.horizontal.prefabs.push_back(data);
        hCount++;
    }
    // Generate 180° rotated variants
    for (int i = 0; i < hCount; i++)
    {
        TilesonMapData *rot = RotatePrefab(corridorPool.horizontal.prefabs[i], 180, 180);
        corridorPool.horizontal.prefabs.push_back(rot);
    }

    corridorPool.loaded = true;
}

void UnloadCorridorPool()
{
    for (auto *p : corridorPool.vertical.prefabs)
        UnloadPrefab(p);
    for (auto *p : corridorPool.horizontal.prefabs)
        UnloadPrefab(p);
    corridorPool.vertical.prefabs.clear();
    corridorPool.horizontal.prefabs.clear();
    corridorPool.loaded = false;
}

void StampCorridor(const MapObject &exitObj, int slotCol, int slotRow)
{
    int exitTileX = (int)(exitObj.bounds.x / WG_TILE_SIZE);
    int exitTileY = (int)(exitObj.bounds.y / WG_TILE_SIZE);

    int exitTypeHash = 0;
    if (exitObj.type == "exit_north")
        exitTypeHash = 1;
    else if (exitObj.type == "exit_south")
        exitTypeHash = 2;
    else if (exitObj.type == "exit_east")
        exitTypeHash = 3;
    else if (exitObj.type == "exit_west")
        exitTypeHash = 4;

    TilesonMapData *vertical = GetRandomCorridor(corridorPool.vertical, exitTileX, exitTileY, exitTypeHash);
    TilesonMapData *horizontal = GetRandomCorridor(corridorPool.horizontal, exitTileX, exitTileY, exitTypeHash);
    if (!vertical || !horizontal)
        return;

    if (tilesonMap->layerCount < WG_CORRIDOR_LAYER_START + 2)
        ExpandCanvasLayers((WG_CORRIDOR_LAYER_START + 2) - tilesonMap->layerCount);

    if (exitObj.type == "exit_north")
    {
        int border = slotRow * WG_CELL_TILES;
        TraceLog(LOG_INFO, "StampCorridor: exit=%s stampCount=%d", exitObj.type.c_str(), exitTileY - border);
        for (int i = 1; i <= exitTileY - border; i++)
            StampMap(vertical, exitTileX - 3, exitTileY - i, WG_CORRIDOR_LAYER_START);
    }
    else if (exitObj.type == "exit_south")
    {
        int border = slotRow * WG_CELL_TILES + WG_CELL_TILES - 1;
        TraceLog(LOG_INFO, "StampCorridor: exit=%s stampCount=%d", exitObj.type.c_str(), border - exitTileY);
        for (int i = 1; i <= border - exitTileY; i++)
            StampMap(vertical, exitTileX - 3, exitTileY + i, WG_CORRIDOR_LAYER_START);
    }
    else if (exitObj.type == "exit_east")
    {
        int border = slotCol * WG_CELL_TILES + WG_CELL_TILES - 1;
        TraceLog(LOG_INFO, "StampCorridor: exit=%s stampCount=%d", exitObj.type.c_str(), border - exitTileX);
        for (int i = 1; i <= border - exitTileX; i++)
            StampMap(horizontal, exitTileX + i, exitTileY - 3, WG_CORRIDOR_LAYER_START);
    }
    else if (exitObj.type == "exit_west")
    {
        int border = slotCol * WG_CELL_TILES;
        TraceLog(LOG_INFO, "StampCorridor: exit=%s stampCount=%d", exitObj.type.c_str(), exitTileX - border);
        for (int i = 1; i <= exitTileX - border; i++)
            StampMap(horizontal, exitTileX - i, exitTileY - 3, WG_CORRIDOR_LAYER_START);
    }
}
