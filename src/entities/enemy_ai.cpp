#include "enemy_ai.h"
#include "map.h"
#include "mapLogic.h"
#include "../lib/raylib/include/raymath.h"
#include <queue>

FlowField globalFlowField;

void FlowField::Invalidate()
{
    isReady_       = false;
    lastGoalTile_  = {-1, -1};
    rebuildCooldown_ = 0.f;
}

void FlowField::Update(Vector2 playerWorld, int mapWidth, int mapHeight)
{
    // throttle — jangan rebuild lebih dari sekali per FLOW_FIELD_REBUILD_COOLDOWN detik
    if (rebuildCooldown_ > 0.f)
    {
        rebuildCooldown_ -= GetFrameTime();
        return;
    }

    // cek apakah player pindah tile sejak build terakhir
    Vector2 currentTile = {
        floorf(playerWorld.x / FLOW_FIELD_TILE_SIZE),
        floorf(playerWorld.y / FLOW_FIELD_TILE_SIZE)
    };

    if (currentTile.x == lastGoalTile_.x && currentTile.y == lastGoalTile_.y)
        return;

    Build(playerWorld, mapWidth, mapHeight);
    rebuildCooldown_ = FLOW_FIELD_REBUILD_COOLDOWN;
}

void FlowField::Build(Vector2 goalWorld, int mapWidth, int mapHeight)
{
    gridWidth_  = mapWidth;
    gridHeight_ = mapHeight;

    // reset grid
    grid_.assign(gridHeight_, std::vector<Cell>(gridWidth_));

    int goalX = Clamp((int)(goalWorld.x / FLOW_FIELD_TILE_SIZE), 0, gridWidth_ - 1);
    int goalY = Clamp((int)(goalWorld.y / FLOW_FIELD_TILE_SIZE), 0, gridHeight_ - 1);

    // hitung batas area aktif
    int startX = std::max(0, goalX - FLOW_FIELD_RADIUS);
    int startY = std::max(0, goalY - FLOW_FIELD_RADIUS);
    int endX   = std::min(gridWidth_ - 1,  goalX + FLOW_FIELD_RADIUS);
    int endY   = std::min(gridHeight_ - 1, goalY + FLOW_FIELD_RADIUS);

     // mark walkable hanya dalam area aktif
    for (int y = startY; y <= endY; y++)
    {
        for (int x = startX; x <= endX; x++)
        {
            Vector2 tileCenter = {
                x * FLOW_FIELD_TILE_SIZE + FLOW_FIELD_TILE_SIZE * 0.5f,
                y * FLOW_FIELD_TILE_SIZE + FLOW_FIELD_TILE_SIZE * 0.5f
            };
            grid_[y][x].walkable = IsPositionSafe(tileCenter, 1.f, 1.f, 0.f, 0.f);
        }
    }

    lastGoalTile_ = {(float)goalX, (float)goalY};

    BFS(goalX, goalY, startX, startY, endX, endY);

    isReady_ = true;
}

void FlowField::BFS(int goalX, int goalY, int startX, int startY, int endX, int endY)
{
    std::vector<std::vector<int>> dist(gridHeight_, std::vector<int>(gridWidth_, -1));
    std::queue<std::pair<int,int>> queue;

    dist[goalY][goalX] = 0;
    queue.push({goalX, goalY});

    const int dx[] = {0, 0, 1, -1, 1, -1,  1, -1};
    const int dy[] = {1,-1, 0,  0, 1,  1, -1, -1};

    while (!queue.empty())
    {
        auto [cx, cy] = queue.front();
        queue.pop();

        for (int i = 0; i < 8; i++)
        {
            int nx = cx + dx[i];
            int ny = cy + dy[i];

            // skip tile di luar area aktif
            if (nx < startX || nx > endX || ny < startY || ny > endY) continue;
            if (!grid_[ny][nx].walkable)  continue;
            if (dist[ny][nx] != -1)       continue;

            dist[ny][nx] = dist[cy][cx] + 1;
            queue.push({nx, ny});
        }
    }

    for (int y = startY; y <= endY; y++)
    {
        for (int x = startX; x <= endX; x++)
        {
            if (!grid_[y][x].walkable || dist[y][x] == -1)
            {
                grid_[y][x].reached   = false;
                grid_[y][x].direction = {0, 0};
                continue;
            }

            grid_[y][x].reached = true;

            int bestDist    = dist[y][x];
            Vector2 bestDir = {0, 0};

            for (int i = 0; i < 8; i++)
            {
                int nx = x + dx[i];
                int ny = y + dy[i];

                if (nx < startX || nx > endX || ny < startY || ny > endY) continue;
                if (dist[ny][nx] == -1) continue;

                if (dist[ny][nx] < bestDist)
                {
                    bestDist = dist[ny][nx];
                    bestDir  = {(float)dx[i], (float)dy[i]};
                }
            }

            grid_[y][x].direction = Vector2Normalize(bestDir);
        }
    }
}

Vector2 FlowField::GetDirection(Vector2 worldPos) const
{
    if (!isReady_) return {0, 0};

    int x = (int)(worldPos.x / FLOW_FIELD_TILE_SIZE);
    int y = (int)(worldPos.y / FLOW_FIELD_TILE_SIZE);

    if (!IsValidTile(x, y))          return {0, 0};
    if (!grid_[y][x].reached)        return {0, 0};

    return grid_[y][x].direction;
}

bool FlowField::IsValidTile(int x, int y) const
{
    return x >= 0 && x < gridWidth_ && y >= 0 && y < gridHeight_;
}