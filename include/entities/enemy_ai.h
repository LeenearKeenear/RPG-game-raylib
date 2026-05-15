#pragma once

#include "../lib/raylib/include/raylib.h"
#include "tiles.h"
#include <vector>

/*==============================================================================
 * FlowField
 *==============================================================================*/
constexpr int FLOW_FIELD_TILE_SIZE = TILE_SIZE;
constexpr float FLOW_FIELD_REBUILD_COOLDOWN = 0.1f; // throttle max 10x/detik
constexpr int FLOW_FIELD_RADIUS = 15;               // area aktif sekitar player (dalam satuan tile)

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

private:
    struct Cell
    {
        Vector2 direction = {0, 0}; // arah normalized menuju goal
        bool walkable = false;      // false jika tile ini obstacle
        bool reached = false;       // false jika tile tidak reachable dari goal
    };

    std::vector<std::vector<Cell>> grid_; // grid_[y][x]
    int gridWidth_ = 0;
    int gridHeight_ = 0;
    bool isReady_ = false;

    // throttle & dirty tracking
    Vector2 lastGoalTile_ = {-1, -1}; // tile terakhir saat build dilakukan
    float rebuildCooldown_ = 0.f;     // sisa waktu cooldown rebuild

    bool IsValidTile(int x, int y) const;
    bool IsTileWalkable(int tileX, int tileY) const;                            // pakai IsPositionSafe
    void BFS(int goalX, int goalY, int startX, int startY, int endX, int endY); // isi direction tiap cell dari goal
};

extern FlowField globalFlowField;