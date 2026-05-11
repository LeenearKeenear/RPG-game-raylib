#include "enemy_ai.h"
#include "enemy.h"
#include "map.h"
#include "mapLogic.h"
#include "player.h"
#include "../lib/raylib/include/raymath.h"
#include <queue>
#include <cfloat>
#include <limits>
#include <tuple>

FlowField globalFlowField;
std::unordered_map<int, SpawnFlowFieldEntry> spawnFlowFields;
std::queue<int> spawnFlowFieldRebuildQueue;

void FlowField::Invalidate()
{
    isReady_ = false;
    lastGoalTile_ = {-1, -1};
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
        floorf(playerWorld.y / FLOW_FIELD_TILE_SIZE)};

    if (currentTile.x == lastGoalTile_.x && currentTile.y == lastGoalTile_.y)
        return;

    Build(playerWorld, mapWidth, mapHeight, FLOW_FIELD_PLAYER_RADIUS);
    rebuildCooldown_ = FLOW_FIELD_REBUILD_COOLDOWN;
}

void FlowField::Build(Vector2 goalWorld, int mapWidth, int mapHeight, int radius)
{
    gridWidth_ = mapWidth;
    gridHeight_ = mapHeight;

    // reset grid
    grid_.assign(gridHeight_, std::vector<Cell>(gridWidth_));

    int goalX = Clamp((int)(goalWorld.x / FLOW_FIELD_TILE_SIZE), 0, gridWidth_ - 1);
    int goalY = Clamp((int)(goalWorld.y / FLOW_FIELD_TILE_SIZE), 0, gridHeight_ - 1);

    // hitung batas area aktif
    int startX = std::max(0, goalX - radius);
    int startY = std::max(0, goalY - radius);
    int endX = std::min(gridWidth_ - 1, goalX + radius);
    int endY = std::min(gridHeight_ - 1, goalY + radius);

    // mark walkable hanya dalam area aktif
    for (int y = startY; y <= endY; y++)
    {
        for (int x = startX; x <= endX; x++)
        {
            Vector2 tileCenter = {
                x * FLOW_FIELD_TILE_SIZE + FLOW_FIELD_CENTER_OFFSET,
                y * FLOW_FIELD_TILE_SIZE + FLOW_FIELD_CENTER_OFFSET};
            grid_[y][x].walkable = IsPositionSafe(tileCenter, 1.f, 1.f, 0.f, 0.f);
            if (grid_[y][x].walkable)
                grid_[y][x].cost = ComputeTileCost(x, y);
        }
    }

    lastGoalTile_ = {(float)goalX, (float)goalY};

    Dijkstra(goalX, goalY, startX, startY, endX, endY);

    isReady_ = true;
}

float FlowField::ComputeTileCost(int x, int y)
{
    float cost = FLOW_FIELD_CARDINAL_COST;

    // sample 4 arah cardinal sejauh 1 tile
    const int dx[] = {1, -1, 0, 0};
    const int dy[] = {0, 0, 1, -1};

    for (int i = 0; i < 4; i++)
    {
        Vector2 samplePos = {
            (x + dx[i]) * FLOW_FIELD_TILE_SIZE + FLOW_FIELD_CENTER_OFFSET,
            (y + dy[i]) * FLOW_FIELD_TILE_SIZE + FLOW_FIELD_CENTER_OFFSET};
        if (!IsPositionSafe(samplePos, 1.f, 1.f, 0.f, 0.f))
        {
            cost += FLOW_FIELD_OBSTACLE_PENALTY;
            break; // cukup satu tetangga obstacle = kena penalty
        }
    }

    return cost;
}

void FlowField::Dijkstra(int goalX, int goalY, int startX, int startY, int endX, int endY)
{
    std::vector<std::vector<float>> dist(gridHeight_,
                                         std::vector<float>(gridWidth_, std::numeric_limits<float>::max()));

    // {cost, x, y}
    using T = std::tuple<float, int, int>;
    std::priority_queue<T, std::vector<T>, std::greater<T>> pq;

    dist[goalY][goalX] = 0.f;
    pq.push({0.f, goalX, goalY});

    const int dx[] = {0, 0, 1, -1, 1, -1, 1, -1};
    const int dy[] = {1, -1, 0, 0, 1, 1, -1, -1};
    const float moveCost[] = {
        FLOW_FIELD_CARDINAL_COST, FLOW_FIELD_CARDINAL_COST,
        FLOW_FIELD_CARDINAL_COST, FLOW_FIELD_CARDINAL_COST,
        FLOW_FIELD_DIAGONAL_COST, FLOW_FIELD_DIAGONAL_COST,
        FLOW_FIELD_DIAGONAL_COST, FLOW_FIELD_DIAGONAL_COST};

    while (!pq.empty())
    {
        auto [d, cx, cy] = pq.top();
        pq.pop();

        if (d > dist[cy][cx])
            continue; // stale entry

        for (int i = 0; i < 8; i++)
        {
            int nx = cx + dx[i];
            int ny = cy + dy[i];

            if (nx < startX || nx > endX || ny < startY || ny > endY)
                continue;
            if (!grid_[ny][nx].walkable)
                continue;

            float newDist = dist[cy][cx] + moveCost[i] + grid_[ny][nx].cost;
            if (newDist < dist[ny][nx])
            {
                dist[ny][nx] = newDist;
                pq.push({newDist, nx, ny});
            }
        }
    }

    // assign direction — sama kayak BFS, cari neighbor dengan dist terkecil
    for (int y = startY; y <= endY; y++)
    {
        for (int x = startX; x <= endX; x++)
        {
            if (!grid_[y][x].walkable || dist[y][x] == std::numeric_limits<float>::max())
            {
                grid_[y][x].reached = false;
                grid_[y][x].direction = {0, 0};
                continue;
            }

            grid_[y][x].reached = true;

            float bestDist = dist[y][x];
            Vector2 bestDir = {0, 0};

            for (int i = 0; i < 8; i++)
            {
                int nx = x + dx[i];
                int ny = y + dy[i];

                if (nx < startX || nx > endX || ny < startY || ny > endY)
                    continue;
                if (dist[ny][nx] == std::numeric_limits<float>::max())
                    continue;

                if (dist[ny][nx] < bestDist)
                {
                    bestDist = dist[ny][nx];
                    bestDir = {(float)dx[i], (float)dy[i]};
                }
            }

            grid_[y][x].direction = Vector2Normalize(bestDir);
        }
    }
}

Vector2 FlowField::GetDirection(Vector2 worldPos) const
{
    if (!isReady_)
        return {0, 0};

    int x = (int)(worldPos.x / FLOW_FIELD_TILE_SIZE);
    int y = (int)(worldPos.y / FLOW_FIELD_TILE_SIZE);

    if (!IsValidTile(x, y))
        return {0, 0};
    if (!grid_[y][x].reached)
        return {0, 0};

    return grid_[y][x].direction;
}

bool FlowField::IsValidTile(int x, int y) const
{
    return x >= 0 && x < gridWidth_ && y >= 0 && y < gridHeight_;
}

float FlowField::GetCost(Vector2 worldPos) const
{
    int x = (int)(worldPos.x / FLOW_FIELD_TILE_SIZE);
    int y = (int)(worldPos.y / FLOW_FIELD_TILE_SIZE);
    if (!IsValidTile(x, y))
        return FLT_MAX;
    if (!grid_[y][x].reached)
        return FLT_MAX;
    return grid_[y][x].cost;
}

// File-local helper — bangun obstacle list buat raycast
std::vector<MapObject> BuildObstacleList()
{
    std::vector<MapObject> result;

    TiledHelper::CollisionResult col;
    if (TiledHelperFunction.TryGetCollisionByLayerName(COLLISION_LAYER_NAME, col))
    {
        for (auto &rect : col.rects)
        {
            MapObject obj;
            obj.bounds = rect;
            obj.hasPolygon = false;
            result.push_back(obj);
        }
        for (auto &poly : col.polygons)
        {
            MapObject obj;
            obj.polygonPoints = poly;
            obj.hasPolygon = true;
            result.push_back(obj);
        }
    }

    auto appendType = [&](const char *type)
    {
        for (auto *p : TilesonGetObjectsByType(type))
            if (p)
                result.push_back(*p);
    };

    appendType(CHEST_TYPE_OBJECT_NAME);
    appendType(BOMB_TYPE_OBJECT_NAME);

    return result;
}

Vector2 EnemySteering::EvaluateGrid(const SteeringContext &ctx, Vector2 flowDir, SteeringMode mode)
{
    float bestScore = -FLT_MAX;
    Vector2 bestDir = flowDir;

    SteeringTarget = Vector2Add(ctx.Position, Vector2Scale(flowDir, FLOW_FIELD_TILE_SIZE));

    Vector2 currentFlowTile = {
        floorf(ctx.Position.x / FLOW_FIELD_TILE_SIZE),
        floorf(ctx.Position.y / FLOW_FIELD_TILE_SIZE)};

    for (int dy = -STEERING_GRID_RADIUS; dy <= STEERING_GRID_RADIUS; dy++)
    {
        for (int dx = -STEERING_GRID_RADIUS; dx <= STEERING_GRID_RADIUS; dx++)
        {
            if (dx == 0 && dy == 0)
                continue;

            int tx = (int)currentFlowTile.x + dx;
            int ty = (int)currentFlowTile.y + dy;

            Vector2 tileCenter = {
                (float)tx * TILE_SIZE + ctx.TileCenterOffset,
                (float)ty * TILE_SIZE + ctx.TileCenterOffset};

            if (!IsPositionSafe(tileCenter, ctx.HitBoxValue, ctx.HitBoxValue, ctx.OffsetValue, ctx.OffsetValue))
                continue;

            if (dx != 0 && dy != 0)
            {
                Vector2 ortho1 = {(float)(tx - dx) * TILE_SIZE + ctx.TileCenterOffset,
                                  (float)ty * TILE_SIZE + ctx.TileCenterOffset};
                Vector2 ortho2 = {(float)tx * TILE_SIZE + ctx.TileCenterOffset,
                                  (float)(ty - dy) * TILE_SIZE + ctx.TileCenterOffset};
                if (!IsPositionSafe(ortho1, ctx.HitBoxValue, ctx.HitBoxValue, ctx.OffsetValue, ctx.OffsetValue) ||
                    !IsPositionSafe(ortho2, ctx.HitBoxValue, ctx.HitBoxValue, ctx.OffsetValue, ctx.OffsetValue))
                    continue;
            }

            Vector2 toTile = Vector2Normalize({tileCenter.x - ctx.Position.x,
                                               tileCenter.y - ctx.Position.y});

            if (SteeringFlipCount >= MaxSteeringFlipCount / 2)
            {
                float cost = (mode == STEERING_CHASE)
                                 ? globalFlowField.GetCost(tileCenter)
                                 : ctx.ReturnFlowField->GetCost(tileCenter);

                float distToPlayer = Vector2Distance(tileCenter, ctx.PlayerCenter);
                if (mode == STEERING_CHASE && distToPlayer > ctx.DetectionRange)
                    continue;

                float score = -cost;
                if (score > bestScore)
                {
                    bestScore = score;
                    bestDir = toTile;
                    SteeringTarget = tileCenter;
                }
                continue;
            }

            float dot = Vector2DotProduct(toTile, flowDir);
            float momentum = Vector2DotProduct(toTile, SteeringDir) * ScoreMultiplier;
            float score = dot + momentum;

            if (score > bestScore)
            {
                bestScore = score;
                bestDir = toTile;
                SteeringTarget = tileCenter;
            }
        }
    }

    return bestDir;
}

void EnemySteering::ApplyAntiFlip(Vector2 bestDir, Vector2 prevDir)
{
    float dirChange = Vector2DotProduct(bestDir, prevDir);
    if (dirChange < 0.f)
    {
        SteeringFlipCount++;
        SteeringFlipTimer = SteeringFlipeTimerWindow;
    }

    SteeringFlipTimer -= GetFrameTime();
    if (SteeringFlipTimer <= 0.f)
        SteeringFlipCount = 0;
}

Vector2 EnemySteering::Compute(SteeringMode mode, const SteeringContext &ctx, RayCast &ray)
{
    Vector2 flowDir;
    if (mode == STEERING_CHASE)
        flowDir = globalFlowField.GetDirection(ctx.Position);
    else if (mode == STEERING_RETURN)
        flowDir = ctx.ReturnFlowField->GetDirection(ctx.Position);
    else
        flowDir = globalFlowField.GetDirection(ctx.Position);

    if (Vector2LengthSqr(flowDir) < 0.001f)
        return flowDir;

    Vector2 currentFlowTile = {
        floorf(ctx.Position.x / FLOW_FIELD_TILE_SIZE),
        floorf(ctx.Position.y / FLOW_FIELD_TILE_SIZE)};

    Vector2 rayDir = (Vector2LengthSqr(ctx.Velocity) > 0.001f)
                         ? Vector2Normalize(ctx.Velocity)
                         : flowDir;

    if (Vector2LengthSqr(SteeringDir) < 0.001f)
        SteeringDir = flowDir;

    auto obstacles = BuildObstacleList();
    ray.Cast(ctx.Position, rayDir, ctx.rayLength, obstacles);

    SteeringCooldown -= GetFrameTime();

    bool tileChanged = (currentFlowTile.x != LastFlowTile.x ||
                        currentFlowTile.y != LastFlowTile.y);

    if (!tileChanged && SteeringCooldown > 0.f)
        return SteeringDir;

    LastFlowTile = currentFlowTile;
    SteeringCooldown = 0.3f; // throtle buat update steering per 0.3 detik

    Vector2 prevDir = SteeringDir;
    Vector2 bestDir = EvaluateGrid(ctx, flowDir, mode);

    SteeringDir = bestDir;
    ApplyAntiFlip(bestDir, prevDir);

    return SteeringDir;
}

bool EnemySteering::IsPlayerInRange(const SteeringContext &ctx, RayCast &ray)
{
    float dist = Vector2Distance(ctx.Position, ctx.PlayerCenter);
    if (dist > ctx.rayLength)
        return false;

    Vector2 dir = Vector2Normalize(Vector2Subtract(ctx.PlayerCenter, ctx.Position));
    auto obstacles = BuildObstacleList();
    RayHitResult hit = ray.Cast(ctx.Position, dir, dist, obstacles);

    return !hit.hit;
}

bool EnemySteering::IsInRangeDebug(Vector2 enemyCenter, Vector2 playerCenter, float rayLength)
{
    return Vector2Distance(enemyCenter, playerCenter) <= rayLength;
}

void BuildSpawnFlowFields(Vector2 spawnPos, int objId, int mapWidth, int mapHeight)
{
    if (spawnFlowFields.count(objId))
        return;

    spawnFlowFields[objId].spawnPos = spawnPos;
    spawnFlowFields[objId].field.Build(spawnPos, mapWidth, mapHeight, FLOW_FIELD_RETURN_RADIUS);
}

FlowField *FindNearestSpawnFlowField(Vector2 position)
{
    FlowField *best = nullptr;
    float bestDist = FLT_MAX;

    for (auto &[id, ff] : spawnFlowFields)
    {
        if (!ff.field.IsReady())
            continue;
        if (ff.field.GetDirection(position).x == 0 && ff.field.GetDirection(position).y == 0)
            continue;

        // pakai goal tile sebagai proxy jarak
        Vector2 dir = ff.field.GetDirection(position);
        if (Vector2LengthSqr(dir) < 0.001f)
            continue;

        // cari berdasarkan cost — makin kecil makin dekat ke goal
        float cost = ff.field.GetCost(position);
        if (cost < bestDist)
        {
            bestDist = cost;
            best = &ff.field;
        }
    }

    return best; // nullptr kalau semua unreachable
}

void RebuildAllSpawnFlowFields()
{
    for (auto &[id, entry] : spawnFlowFields)
        entry.field.Build(entry.spawnPos, tilesonMap->width, tilesonMap->height, FLOW_FIELD_RETURN_RADIUS);
}

void MarkSpawnFlowFieldsDirty(Vector2 explosionPos)
{
    for (auto &[id, entry] : spawnFlowFields)
    {
        float dist = Vector2Distance(entry.spawnPos, explosionPos);
        if (dist < FLOW_FIELD_RETURN_RADIUS * FLOW_FIELD_TILE_SIZE)
            spawnFlowFieldRebuildQueue.push(id);
    }
}