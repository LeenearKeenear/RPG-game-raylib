#pragma once
#include "../lib/raylib/include/raylib.h"
#include "../lib/tileson/tileson.hpp"
#include "screen.h"
#include <string>
#include <vector>

struct MapObject
{
    std::string name;
    std::string type;
    Rectangle bounds;
    std::map<std::string, tson::Property> properties;
};

using TilesonMapData = struct
{
    int width;
    int height;
    int layerCount;
    int **tiles;
    Texture2D tilesetTexture;
    int tilesetCols;
    int tilesetSpacing;
    int tilesetFirstgid;
    std::vector<MapObject> Objects;
};

// query functions
std::vector<MapObject> TilesonGetObjectsByType(const std::string &type);
MapObject *TilesonGetObjectByName(const std::string &name);

extern TilesonMapData *tilesonMap;

void TilesonLoadMap(const char *mapPath);
void TilesonUnloadMap();
void TilesonInit(GameState *state);
void TilesonRender(GameState *state);
void TilesonDebugDraw();
