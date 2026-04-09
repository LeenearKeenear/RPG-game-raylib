#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/screen.h"
#include "../include/tileson_map.h"
#include "../include/player.h"
#include <memory>
#include <string>

TilesonMapData *tilesonMap = nullptr;
static std::unique_ptr<tson::Map> parsedMap = nullptr;

void TilesonLoadMap(const char *mapPath)
{
    tson::Tileson t;
    parsedMap = t.parse(mapPath);

    if (parsedMap->getStatus() != tson::ParseStatus::OK)
    {
        TraceLog(LOG_ERROR, "Tileson: Failed to parse map: %s", parsedMap->getStatusMessage().c_str());
        return;
    }

    tilesonMap = new TilesonMapData();
    tilesonMap->width = parsedMap->getWidth();
    tilesonMap->height = parsedMap->getHeight();
    tilesonMap->tiles = new int[tilesonMap->width * tilesonMap->height];

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
                {
                    tilesonMap->tiles[y * tilesonMap->width + x] = (int)tile->getId();
                }
            }
        }
    }

    tson::Tileset *tileset = parsedMap->getTileset("demo-tileset");
    if (tileset != nullptr)
    {
        std::string imagePath = tileset->getImagePath().u8string();
        Image img = LoadImage(imagePath.c_str());
        tilesonMap->tilesetTexture = LoadTextureFromImage(img);
        UnloadImage(img);
        TraceLog(LOG_INFO, "Tileson: Loaded tileset texture: %s", imagePath.c_str());
    }
    else
    {
        TraceLog(LOG_WARNING, "Tileson: Tileset 'demo-tileset' not found in map");
    }

    TraceLog(LOG_INFO, "Tileson: Map loaded - %dx%d tiles", tilesonMap->width, tilesonMap->height);
}

void TilesonUnloadMap()
{
    if (tilesonMap != nullptr)
    {
        if (tilesonMap->tiles != nullptr)
        {
            delete[] tilesonMap->tiles;
        }
        if (tilesonMap->tilesetTexture.id != 0)
        {
            UnloadTexture(tilesonMap->tilesetTexture);
        }
        delete tilesonMap;
        tilesonMap = nullptr;
    }
    parsedMap.reset();
}

void TilesonInit(GameState *state)
{
    TilesonLoadMap("content/simple_map.json");
}

void TilesonRender(GameState *state)
{
    if (tilesonMap == nullptr || tilesonMap->tilesetTexture.id == 0)
        return;

    BeginMode2D(camera);

    for (int y = 0; y < tilesonMap->height; y++)
    {
        for (int x = 0; x < tilesonMap->width; x++)
        {
            int tileId = tilesonMap->tiles[y * tilesonMap->width + x];
            if (tileId == 0)
                continue;

            int tilesetCols = 8;
            int srcX = (tileId % tilesetCols) * TILE_SIZE;
            int srcY = (tileId / tilesetCols) * TILE_SIZE;

            Rectangle srcRec = {(float)srcX, (float)srcY, (float)TILE_SIZE, (float)TILE_SIZE};
            Rectangle dstRec = {(float)(x * TILE_SIZE), (float)(y * TILE_SIZE), (float)TILE_SIZE, (float)TILE_SIZE};

            DrawTexturePro(tilesonMap->tilesetTexture, srcRec, dstRec, (Vector2){0, 0}, 0.0f, WHITE);
        }
    }

    EndMode2D();
}

void TilesonDebugDraw()
{
    if (tilesonMap == nullptr)
        return;

    DrawRectangle(5, 5, 250, 80, DARKGRAY);
    DrawRectangleLines(5, 5, 250, 80, WHITE);
    DrawText("Tileson Demo Map", 15, 10, 20, YELLOW);
    DrawText(TextFormat("Size: %dx%d", tilesonMap->width, tilesonMap->height), 15, 30, 18, WHITE);
    DrawText(TextFormat("Tileset: %s", tilesonMap->tilesetTexture.id != 0 ? "Loaded" : "Not loaded"), 15, 50, 18, WHITE);
    DrawText("Source: content/simple_map.json", 15, 70, 14, GRAY);
}
