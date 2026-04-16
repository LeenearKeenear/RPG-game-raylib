#pragma once
#include "../lib/raylib/include/raylib.h"
#include "../lib/tileson/tileson.hpp"
#include "screen.h"
#include <string>
#include <vector>
#include <map>

// camera
extern Camera2D camera;

// ================================================================
// Tileson Map System
// ================================================================

// representasi satu object dari object layer Tiled
// bisa berupa collision rect, spawn point, door, dll
struct MapObject
{
    std::string name;
    std::string type;
    std::string layerName;              // nama object layer asal di Tiled
    Rectangle bounds;
    std::vector<Vector2> polygonPoints; // titik polygon/polyline dalam world space
    bool hasPolygon = false;            // true kalau object ini punya polygon custom
    std::map<std::string, tson::Property> properties;
};

// info satu tileset — texture + metadata buat render
struct TilesetInfo
{
    Texture2D texture;
    int cols;
    int spacing;
    int firstgid;
    int lastgid; // firstgid tileset berikutnya - 1, dipake buat cari tileset yang bener pas render
};

// semua data map yang udah di-parse dari JSON Tiled
struct TilesonMapData
{
    int width;
    int height;
    int layerCount;
    int **tiles;
    std::vector<TilesetInfo> tilesets; // ganti single texture jadi vector
    std::vector<MapObject> Objects;
};

extern TilesonMapData *tilesonMap;

// hasil kalkulasi frustum: range index tile yang visible di layar
struct TileRange
{
    int minX; // kolom tile paling kiri yang visible
    int minY; // baris tile paling atas yang visible
    int maxX; // kolom tile paling kanan yang visible (exclusive)
    int maxY; // baris tile paling bawah yang visible (exclusive)
};

// data yang bisa dibaca oleh sistem debug
extern int lastTilesRendered;
extern TileRange currentVisibleRange;

// nama layer & object di Tiled — sesuaiin kalau beda
// #define COLLISION_LAYER_NAME "map_bound"
#define COLLISION_LAYER_NAME "obstacle" // penulisan define untuk layer
#define OBJECT_LAYER_NAME "object"
#define SPAWN_OBJECT_NAME "spawn" // penulisan define untuk object name
#define DOOR_TYPE_OBJECT_NAME "pass" // penulisan define untuk type object name

// ================================================================
// Functions
// ================================================================

// load, render, unload map dari JSON Tiled
void LoadMap(const char *mapPath);
void RenderMap(void);
void UnloadMap(void);
void InitMap(void);
void SwitchMap(const char *newMapPath, const char *targetSpawnName);



// query object dari object layer Tiled
std::vector<MapObject> TilesonGetObjectsByLayerName(const std::string &layerName);
std::vector<MapObject> TilesonGetObjectsByType(const std::string &type);
MapObject *TilesonGetObjectByName(const std::string &name);
