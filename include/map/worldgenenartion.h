#pragma once
#include "map.h"
#include "tiles.h"
#include <vector>
#include <string>

/*==============================================================================
 * Constants
 *==============================================================================*/

constexpr int WG_GRID_SIZE = 4;
constexpr int WG_CELL_TILES = 41;
constexpr int WG_CANVAS_TILES = 164;
constexpr int WG_TILE_SIZE = TILE_SIZE;

constexpr const char *SLOT_WORLDGEN_LAYER_NAME = "slot_worldgen";

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

struct CorridorTemplate
{
    int direction;
    int length;
};

struct WorldCell
{
    CellType type;
    int exitMask;
    RoomTemplate *roomTemplate;
};

/*==============================================================================
 * Functions
 *==============================================================================*/

std::vector<MapObject *> GetWorldGenSlots();
void PreFabLoadMap(const char *mapPath, TilesonMapData *target);
void UnloadPrefab(TilesonMapData* prefab);
void ExpandCanvasLayers(int extraLayers);
void StampPrefabToSlot(TilesonMapData *prefab, MapObject *slot);
void StampMap(TilesonMapData *source, int offsetX, int offsetY, int targetLayerOffset);