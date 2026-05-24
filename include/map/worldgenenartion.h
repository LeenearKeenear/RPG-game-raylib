#pragma once
#include "map.h"
#include "animation.h"
#include <vector>
#include <string>

/*==============================================================================
 * Constants
 *==============================================================================*/

constexpr int WG_GRID_SIZE = 4;
constexpr int WG_CELL_TILES = 41;
constexpr int WG_CANVAS_TILES = 164;
constexpr int WG_PREFAB_LAYER_START = 2;
constexpr int WG_CORRIDOR_LAYER_START = 4;
constexpr int WG_TILE_SIZE = FRAME_SIZE;

constexpr const char *SLOT_WORLDGEN_LAYER_NAME = "slot_worldgen";
constexpr const char *EXIT_NORTH_TYPE_OBJECT_NAME = "exit_north";
constexpr const char *EXIT_EAST_TYPE_OBJECT_NAME = "exit_east";
constexpr const char *EXIT_SOUTH_TYPE_OBJECT_NAME = "exit_south";
constexpr const char *EXIT_WEST_TYPE_OBJECT_NAME = "exit_west";

/*==============================================================================
 * Enums
 *==============================================================================*/

enum ExitDirection
{
    EXIT_NONE = 0,
    EXIT_NORTH = 1 << 0,
    EXIT_EAST = 1 << 1,
    EXIT_SOUTH = 1 << 2,
    EXIT_WEST = 1 << 3,
};

enum CellType
{
    CELL_EMPTY = 0,
    CELL_START = 1,
    CELL_ENEMY = 2,
    CELL_ENEMY_ELITE = 3,
    CELL_TREASURE = 4,
    CELL_TRADER = 5,
    CELL_FINISH = 6,
    CELL_BOSS = 7,
    CELL_SPECIAL = 8,
};

/*==============================================================================
 * Structs
 *==============================================================================*/

struct RoomTemplate
{
    const char *jsonPath;
    int width;
    int height;
    int exitMask;
};

struct WorldCell
{
    CellType type;
    int exitMask;
    RoomTemplate *roomTemplate;
};

struct WeightedPool
{
    std::vector<TilesonMapData*> prefabs;
    bool loaded = false;
};

struct CorridorPool
{
    WeightedPool vertical;
    WeightedPool horizontal;
    bool loaded = false;
};

extern CorridorPool corridorPool;

/*==============================================================================
 * Functions
 *==============================================================================*/

std::vector<MapObject> GetWorldGenSlots();
std::vector<std::vector<int>> ReshapeTo2D(const std::vector<int> &flatData, int width, int height);
std::vector<int> FlattenTo1D(const std::vector<std::vector<int>> &grid);
std::vector<std::vector<int>> Rotate90CW(const std::vector<std::vector<int>> &grid);
std::vector<std::vector<int>> RotateGrid(const std::vector<std::vector<int>> &grid, int degrees);

void PreFabLoadMap(const char *mapPath, TilesonMapData *target);
void UnloadPrefab(TilesonMapData *prefab);
void ExpandCanvasLayers(int extraLayers);
void StampMap(TilesonMapData *source, int offsetX, int offsetY, int targetLayerOffset);
void StampPrefabToSlot(TilesonMapData *prefab, MapObject *slot);
TilesonMapData *RotateTileLayers(TilesonMapData *source, int degrees);
void RotateObjectLayer(TilesonMapData *source, TilesonMapData *result, int degrees, int tileSize, bool obstacleOnly);
TilesonMapData *RotatePrefab(TilesonMapData *source, int tileDegrees, int objectDegrees);
void StampCorridor(const MapObject &exitObj, int slotCol, int slotRow);

void LoadCorridorPool(const char *basePath);
void UnloadCorridorPool();

extern uint64_t worldSeed;