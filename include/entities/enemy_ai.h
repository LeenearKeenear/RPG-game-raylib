#pragma once

#include "../lib/raylib/include/raylib.h"
#include "enemy.h"
#include "map.h"
#include "mapLogic.h"
#include "tiles.h"
#include <vector>

/*==============================================================================
 * FlowField
 *==============================================================================*/
constexpr int FLOW_FIELD_TILE_SIZE = TILE_SIZE;
constexpr float FLOW_FIELD_CENTER_OFFSET = FLOW_FIELD_TILE_SIZE * 0.5f;
constexpr float FLOW_FIELD_REBUILD_COOLDOWN = 0.3f; // throttle max 10x/detik
constexpr int FLOW_FIELD_RADIUS = 15;               // area aktif sekitar player (dalam satuan tile)
constexpr int STEERING_GRID_RADIUS = 2;             // 1 = 3x3, 2 = 5x5, 3 = 7 x 7, 4 = 9 x 9

constexpr int FLOW_FIELD_OBSTACLE_CHECK_RADIUS = 1; // jarak sample proximity (dalam tile)
constexpr float FLOW_FIELD_OBSTACLE_PENALTY = 5.f;  // cost tambahan tile deket obstacle
constexpr float FLOW_FIELD_DIAGONAL_COST = 1.414f;  // sqrt(2)
constexpr float FLOW_FIELD_CARDINAL_COST = 1.0f;

/**
 * @brief Grid arah menuju player, dihitung sekali dan dibaca semua enemy.
 *
 * Cara kerja:
 * - Build() dipanggil saat player pindah tile atau map baru di-load
 * - GetDirection() dipanggil tiap enemy tiap frame — O(1), tidak ada pathfinding per enemy
 *
 * Koordinat grid: tileX = worldX / FLOW_FIELD_TILE_SIZE
 */
class FlowField
{
public:
    /**
     * @brief Build flow field dari posisi player sebagai tujuan.
     * @param goalWorld Posisi player dalam world space
     * @param mapWidth Lebar map dalam satuan tile
     * @param mapHeight Tinggi map dalam satuan tile
     * @note Pakai BFS dari goal — tile non-walkable di-skip
     */
    void Build(Vector2 goalWorld, int mapWidth, int mapHeight);

    /**
     * @brief Ambil arah gerak untuk posisi world space tertentu.
     * @param worldPos Posisi enemy dalam world space
     * @return Arah normalized menuju player, {0,0} jika tile tidak valid atau tidak reachable
     */
    Vector2 GetDirection(Vector2 worldPos) const;

    /**
     * @brief Cek apakah flow field sudah siap dipakai.
     */
    bool IsReady() const { return isReady_; }

    /**
     * @brief Notify bahwa map baru di-load — paksa rebuild pada Build() berikutnya.
     */
    void Invalidate();

    /**
     * @brief Update dipanggil tiap frame — cek apakah perlu rebuild berdasarkan posisi player.
     * @param playerWorld Posisi player dalam world space
     * @param mapWidth Lebar map dalam satuan tile
     * @param mapHeight Tinggi map dalam satuan tile
     */
    void Update(Vector2 playerWorld, int mapWidth, int mapHeight);

    float GetCost(Vector2 worldPos) const;

private:
    struct Cell
    {
        Vector2 direction = {0, 0}; // arah normalized menuju goal
        bool walkable = false;      // false jika tile ini obstacle
        bool reached = false;       // false jika tile tidak reachable dari goal
        float cost = 1.f;           // base cost, naik kalau deket obstacle
    };

    std::vector<std::vector<Cell>> grid_; // grid_[y][x]
    int gridWidth_ = 0;
    int gridHeight_ = 0;
    bool isReady_ = false;

    // throttle & dirty tracking
    Vector2 lastGoalTile_ = {-1, -1}; // tile terakhir saat build dilakukan
    float rebuildCooldown_ = 0.f;     // sisa waktu cooldown rebuild

    bool IsValidTile(int x, int y) const;
    bool IsTileWalkable(int tileX, int tileY) const;                                 // pakai IsPositionSafe
    void Dijkstra(int goalX, int goalY, int startX, int startY, int endX, int endY); // isi direction tiap cell dari goal
    float ComputeTileCost(int x, int y);
};

// EnemySteering.h

// enum buat jenis behavior movement enemynya
enum SteeringMode
{
    STEERING_CHASE,
    STEERING_RETURN
};

struct SteeringContext
{
    Vector2 Position;
    Vector2 Velocity;
    float HitBoxValue;
    float OffsetValue;
    float TileCenterOffset;
    float DetectionRange;
    float rayLength;
    Vector2 PlayerCenter;
    Vector2 SpawnPoint;
};

class EnemySteering
{
public:
    Vector2 Compute(SteeringMode mode, const SteeringContext &ctx, RayCast &ray);
    bool IsPlayerInRange(const SteeringContext &ctx, RayCast &ray);
    bool IsInRangeDebug(Vector2 enemyCenter, Vector2 playerCenter, float rayLength);

    // state
    Vector2 SteeringDir = {0, 0};
    Vector2 LastFlowTile = {-1, -1};
    Vector2 SteeringTarget = {0.f, 0.f};

    int SteeringFlipCount = 0;
    int MaxSteeringFlipCount = 4;
    float SteeringFlipTimer = 0.f;
    float SteeringFlipeTimerWindow = 0.3f;
    float SteeringCooldown = 0.f;
    float ScoreMultiplier = 0.9f;

private:
    Vector2 EvaluateGrid(const SteeringContext &ctx, Vector2 flowDir, SteeringMode mode);
    void ApplyAntiFlip(Vector2 bestDir, Vector2 prevDir);
};

std::vector<MapObject> BuildObstacleList();

extern FlowField globalFlowField;
extern FlowField returnFlowField;
extern bool returnFlowFieldBuilt;
