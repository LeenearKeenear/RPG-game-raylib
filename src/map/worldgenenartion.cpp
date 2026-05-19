#include "worldgenenartion.h"
#include "mapLogic.h"

extern TilesonMapData *tilesonMap;

std::vector<MapObject *> GetWorldGenSlots()
{
    const auto &lyrs = TilesonGetObjectsByLayerName(SLOT_WORLDGEN_LAYER_NAME);
    return std::vector<MapObject *>(lyrs.begin(), lyrs.end());
}

void PreFabLoadMap(const char *mapPath, TilesonMapData *target)
{
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
        if (TexturesMap[TEXTURE_TILEMAP].id != 0 && imagePath == "assets/textures/tiles.png")
        {
            info.texture = TexturesMap[TEXTURE_TILEMAP];
            TraceLog(LOG_INFO, "Tileson: Reusing cached texture for tiles.png");
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
        tilesonMap->layerTilesetGroup.assign(tilesonMap->layerCount, 0);
        TraceLog(LOG_INFO, "Tileson: Loaded tileset %s (gid %d-%d)", imagePath.c_str(), info.firstgid, info.lastgid);
    }

    TraceLog(LOG_INFO, "Tileson: Map loaded - %dx%d tiles", tilesonMap->width, tilesonMap->height);
}

void UnloadPrefab(TilesonMapData* prefab)
{
    if (!prefab) return;
    for (int i = 0; i < prefab->layerCount; i++)
        delete[] prefab->tiles[i];
    delete[] prefab->tiles;
    delete prefab;
}

void StampMap(TilesonMapData* source, int offsetX, int offsetY, int targetLayerOffset)
{
    // append tileset group dari prefab
    int newGroupIdx = (int)tilesonMap->tilesets.size();
    tilesonMap->tilesets.push_back(source->tilesets[0]);

    // stamp tile, GID gak perlu di-shift
    for (int l = 0; l < source->layerCount; l++)
    {
        int dstLayer = targetLayerOffset + l;
        if (dstLayer >= tilesonMap->layerCount) break;

        // assign layer ke group baru
        tilesonMap->layerTilesetGroup[dstLayer] = newGroupIdx;

        for (int y = 0; y < source->height; y++)
        {
            for (int x = 0; x < source->width; x++)
            {
                int srcTile = source->tiles[l][y * source->width + x];
                if (srcTile == 0) continue;

                int cx = offsetX + x;
                int cy = offsetY + y;
                if (cx >= tilesonMap->width || cy >= tilesonMap->height) continue;

                tilesonMap->tiles[dstLayer][cy * tilesonMap->width + cx] = srcTile;
            }
        }
    }

    // stamp objects
    for (auto& obj : source->Objects)
    {
        MapObject stamped = obj;
        stamped.bounds.x += offsetX * WG_TILE_SIZE;
        stamped.bounds.y += offsetY * WG_TILE_SIZE;
        for (auto& p : stamped.polygonPoints)
        {
            p.x += offsetX * WG_TILE_SIZE;
            p.y += offsetY * WG_TILE_SIZE;
        }
        tilesonMap->Objects.push_back(stamped);
    }
}

// void StampPrefabToSlot(TilesonMapData *prefab, MapObject *slot)
// {
//     // slot bounds dalam pixel, convert ke tile
//     int slotTileX = (int)(slot->bounds.x / WG_TILE_SIZE);
//     int slotTileY = (int)(slot->bounds.y / WG_TILE_SIZE);
//     int slotTileW = (int)(slot->bounds.width / WG_TILE_SIZE);
//     int slotTileH = (int)(slot->bounds.height / WG_TILE_SIZE);

//     // center prefab di dalam slot
//     int offsetX = slotTileX + (slotTileW - prefab->width) / 2;
//     int offsetY = slotTileY + (slotTileH - prefab->height) / 2;

//     StampMap(prefab, offsetX, offsetY);
// }

void ExpandCanvasLayers(int extraLayers)
{
    int newLayerCount = tilesonMap->layerCount + extraLayers;

    // alokasi array baru
    int **newTiles = new int *[newLayerCount];

    // copy pointer layer lama
    for (int i = 0; i < tilesonMap->layerCount; i++)
        newTiles[i] = tilesonMap->tiles[i];

    // alokasi layer baru, zero-init
    for (int i = tilesonMap->layerCount; i < newLayerCount; i++)
        newTiles[i] = new int[tilesonMap->width * tilesonMap->height]();

    // replace
    delete[] tilesonMap->tiles;
    tilesonMap->tiles = newTiles;
    tilesonMap->layerCount = newLayerCount;
}