#include "enemy_ai.h"
#include "enemy.h"
#include "map.h"
#include "mapLogic.h"
#include "../lib/raylib/include/raymath.h"
#include <queue>
#include <cfloat>
#include <limits>
#include <tuple>

FlowField globalFlowField;
FlowField returnFlowField;
bool returnFlowFieldBuilt = false;

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

    Build(playerWorld, mapWidth, mapHeight);
    rebuildCooldown_ = FLOW_FIELD_REBUILD_COOLDOWN;
}

void FlowField::Build(Vector2 goalWorld, int mapWidth, int mapHeight)
{
    gridWidth_ = mapWidth;
    gridHeight_ = mapHeight;

    // reset grid
    grid_.assign(gridHeight_, std::vector<Cell>(gridWidth_));

    int goalX = Clamp((int)(goalWorld.x / FLOW_FIELD_TILE_SIZE), 0, gridWidth_ - 1);
    int goalY = Clamp((int)(goalWorld.y / FLOW_FIELD_TILE_SIZE), 0, gridHeight_ - 1);

    // hitung batas area aktif
    int startX = std::max(0, goalX - FLOW_FIELD_RADIUS);
    int startY = std::max(0, goalY - FLOW_FIELD_RADIUS);
    int endX = std::min(gridWidth_ - 1, goalX + FLOW_FIELD_RADIUS);
    int endY = std::min(gridHeight_ - 1, goalY + FLOW_FIELD_RADIUS);

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

Vector2 Enemy::ComputeSteering(SteeringMode mode)
{
    Vector2 flowDir;
    if (mode == STEERING_CHASE)
        flowDir = globalFlowField.GetDirection(Position);
    else if (mode == STEERING_RETURN)
        flowDir = returnFlowField.GetDirection(Position);
    else
        flowDir = globalFlowField.GetDirection(Position); // fallback

    // kalau flow field belum ready, balik zero
    if (Vector2LengthSqr(flowDir) < 0.001f)
        return flowDir;

    // tile posisi enemy sekarang
    Vector2 currentFlowTile = {
        floorf(Position.x / FLOW_FIELD_TILE_SIZE),
        floorf(Position.y / FLOW_FIELD_TILE_SIZE)};

    // arah raycast — pakai velocity kalau ada, fallback ke flowDir
    Vector2 rayDir = (Vector2LengthSqr(Velocity) > 0.001f)
                         ? Vector2Normalize(Velocity)
                         : flowDir;

    // inisialisasi SteeringDir dengan flowDir kalau belum pernah di-set
    if (Vector2LengthSqr(SteeringDir) < 0.001f)
        SteeringDir = flowDir;

    auto obstacles = BuildObstacleList();
    RayHitResult hit = Ray.Cast(Position, rayDir, rayLength, obstacles);

    SteeringCooldown -= GetFrameTime();

    bool tileChanged = (currentFlowTile.x != LastFlowTile.x ||
                        currentFlowTile.y != LastFlowTile.y);

    // mulus — pakai hasil terakhir
    if (!tileChanged && SteeringCooldown > 0.f)
        return SteeringDir;

    LastFlowTile = currentFlowTile;
    SteeringCooldown = 0.3f; // reset cooldown setelah evaluate

    // --- 5x5 tile evaluation ---
    float bestScore = -FLT_MAX;
    Vector2 bestDir = flowDir;

    SteeringTarget = Vector2Add(Position, Vector2Scale(flowDir, FLOW_FIELD_TILE_SIZE)); // fallback

    for (int dy = -STEERING_GRID_RADIUS; dy <= STEERING_GRID_RADIUS; dy++)
    {
        for (int dx = -STEERING_GRID_RADIUS; dx <= STEERING_GRID_RADIUS; dx++)
        {
            // skip tile enemy sendiri
            if (dx == 0 && dy == 0)
                continue;

            int tx = (int)currentFlowTile.x + dx;
            int ty = (int)currentFlowTile.y + dy;

            Vector2 tileCenter = {
                (float)tx * TILE_SIZE + TileCenterOffset,
                (float)ty * TILE_SIZE + TileCenterOffset};

            if (!IsPositionSafe(tileCenter, HitBoxValue, HitBoxValue,
                                HitboxOffsetX, HitboxOffsetY))
                continue;

            if (dx != 0 && dy != 0)
            {
                Vector2 ortho1 = {
                    (float)(tx - dx) * TILE_SIZE + TileCenterOffset,
                    (float)ty * TILE_SIZE + TileCenterOffset};
                Vector2 ortho2 = {
                    (float)tx * TILE_SIZE + TileCenterOffset,
                    (float)(ty - dy) * TILE_SIZE + TileCenterOffset};
                if (!IsPositionSafe(ortho1, HitBoxValue, HitBoxValue, HitboxOffsetX, HitboxOffsetY) ||
                    !IsPositionSafe(ortho2, HitBoxValue, HitBoxValue, HitboxOffsetX, HitboxOffsetY))
                    continue;
            }

            Vector2 toTile = Vector2Normalize({tileCenter.x - Position.x,
                                               tileCenter.y - Position.y});

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

    Vector2 prevDir = SteeringDir; // simpan dulu
    SteeringDir = bestDir;

    float dirChange = Vector2DotProduct(bestDir, prevDir); // bandingkan baru vs lama
    if (dirChange < 0.f)                                   // arah kebalik
    {
        SteeringFlipCount++;
        SteeringFlipTimer = SteeringFlipeTimerWindow;
    }

    SteeringFlipTimer -= GetFrameTime();
    if (SteeringFlipTimer <= 0.f)
        SteeringFlipCount = 0;

    return SteeringDir;
}

Vector2 Enemy::ComputeSteering(SteeringMode mode, int abc)
{
    Vector2 flowDir;
    if (mode == STEERING_CHASE)
        flowDir = globalFlowField.GetDirection(Position);
    else if (mode == STEERING_RETURN)
        flowDir = returnFlowField.GetDirection(Position);
    else
        flowDir = globalFlowField.GetDirection(Position); // fallback

    // kalau flow field belum ready, balik zero
    if (Vector2LengthSqr(flowDir) < 0.001f)
        return flowDir;

    // tile posisi enemy sekarang
    Vector2 currentFlowTile = {
        floorf(Position.x / FLOW_FIELD_TILE_SIZE),
        floorf(Position.y / FLOW_FIELD_TILE_SIZE)};

    SteeringCooldown -= GetFrameTime();
    bool tileChanged = (currentFlowTile.x != LastFlowTile.x ||
                        currentFlowTile.y != LastFlowTile.y);

    if (!tileChanged && SteeringCooldown > 0.f)
        return SteeringDir;

    LastFlowTile = currentFlowTile;
    SteeringCooldown = 0.3f;

    // --- 8 ray detection ---
    const Vector2 directions8[8] = {
        {0, -1},             // U
        {0.7071f, -0.7071f}, // RU
        {1, 0},              // R
        {0.7071f, 0.7071f},  // RD
        {0, 1},              // D
        {-0.7071f, 0.7071f}, // LD
        {-1, 0},             // L
        {-0.7071f, -0.7071f} // LU
    };

    auto obstacles = BuildObstacleList();
    bool anyHit = false;
    float penalty[8] = {};

    for (int i = 0; i < 8; i++)
    {
        RayHitResult hit = Ray.Cast(Position, directions8[i], rayLength, obstacles);
        if (hit.hit)
        {
            anyHit = true;
            penalty[i] = 1.0f - (hit.distance / rayLength); // makin deket makin tinggi
        }
    }

    // semua clear — gerak normal flow field
    if (!anyHit)
    {
        SteeringDir = flowDir;
        return SteeringDir;
    }

    // ada obstacle — evaluate 4 arah
    const Vector2 directions4[4] = {
        {0, -1}, // U
        {1, 0},  // R
        {0, 1},  // D
        {-1, 0}  // L
    };

    // index mapping directions4 ke directions8
    const int map4to8[4] = {0, 2, 4, 6};

    float bestScore = -FLT_MAX;
    Vector2 bestDir = flowDir;

    for (int i = 0; i < 4; i++)
    {
        float alignment = Vector2DotProduct(directions4[i], flowDir);
        float obs_penalty = penalty[map4to8[i]];
        float score = alignment - obs_penalty;

        if (score > bestScore)
        {
            bestScore = score;
            bestDir = directions4[i];
        }
    }

    SteeringDir = bestDir;
    return SteeringDir;
}

bool Enemy::IsPlayerInRange(Vector2 playerPos)
{
    float dist = Vector2Distance(GetCenter(), playerPos);
    if (dist > rayLength)
        return false;

    Vector2 dir = Vector2Normalize(Vector2Subtract(playerPos, GetCenter()));
    auto obstacles = BuildObstacleList();
    RayHitResult hit = Ray.Cast(GetCenter(), dir, dist, obstacles);

    return !hit.hit;
}