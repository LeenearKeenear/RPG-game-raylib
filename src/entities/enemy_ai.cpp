#include "enemy_ai.h"
#include "enemy.h"
#include "map.h"
#include "mapLogic.h"
#include "../lib/raylib/include/raymath.h"
#include <queue>
#include <cfloat>

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
                x * FLOW_FIELD_TILE_SIZE + FLOW_FIELD_TILE_SIZE * 0.5f,
                y * FLOW_FIELD_TILE_SIZE + FLOW_FIELD_TILE_SIZE * 0.5f};
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
    std::queue<std::pair<int, int>> queue;

    dist[goalY][goalX] = 0;
    queue.push({goalX, goalY});

    const int dx[] = {0, 0, 1, -1, 1, -1, 1, -1};
    const int dy[] = {1, -1, 0, 0, 1, 1, -1, -1};

    while (!queue.empty())
    {
        auto [cx, cy] = queue.front();
        queue.pop();

        for (int i = 0; i < 8; i++)
        {
            int nx = cx + dx[i];
            int ny = cy + dy[i];

            // skip tile di luar area aktif
            if (nx < startX || nx > endX || ny < startY || ny > endY)
                continue;
            if (!grid_[ny][nx].walkable)
                continue;
            if (dist[ny][nx] != -1)
                continue;

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
                grid_[y][x].reached = false;
                grid_[y][x].direction = {0, 0};
                continue;
            }

            grid_[y][x].reached = true;

            int bestDist = dist[y][x];
            Vector2 bestDir = {0, 0};

            for (int i = 0; i < 8; i++)
            {
                int nx = x + dx[i];
                int ny = y + dy[i];

                if (nx < startX || nx > endX || ny < startY || ny > endY)
                    continue;
                if (dist[ny][nx] == -1)
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

    float rayLength = TILE_SIZE * 2.0f;
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
    Vector2 bestDir = flowDir; // fallback ke flow field

    SteeringTarget = Vector2Add(Position, Vector2Scale(flowDir, FLOW_FIELD_TILE_SIZE)); // fallback

    for (int dy = -2; dy <= 2; dy++)
    {
        for (int dx = -2; dx <= 2; dx++)
        {
            // skip tile enemy sendiri
            if (dx == 0 && dy == 0)
                continue;

            int tx = (int)currentFlowTile.x + dx;
            int ty = (int)currentFlowTile.y + dy;

            Vector2 tileCenter = {
                tx * TILE_SIZE + TILE_SIZE * 0.5f,
                ty * TILE_SIZE + TILE_SIZE * 0.5f};

            // cek walkable pakai hitbox enemy
            if (!IsPositionSafe(tileCenter, HitboxWidth, HitboxHeight,
                                HitboxOffsetX, HitboxOffsetY))
                continue;

            // no corner cutting — diagonal harus kedua ortogonal walkable
            if (dx != 0 && dy != 0)
            {
                Vector2 ortho1 = {
                    (tx - dx) * TILE_SIZE + TILE_SIZE * 0.5f,
                    ty * TILE_SIZE + TILE_SIZE * 0.5f};
                Vector2 ortho2 = {
                    tx * TILE_SIZE + TILE_SIZE * 0.5f,
                    (ty - dy) * TILE_SIZE + TILE_SIZE * 0.5f};
                if (!IsPositionSafe(ortho1, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY) ||
                    !IsPositionSafe(ortho2, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY))
                    continue;
            }

            Vector2 toTile = Vector2Normalize({tileCenter.x - Position.x,
                                               tileCenter.y - Position.y});

            float dot = Vector2DotProduct(toTile, flowDir);
            float momentum = Vector2DotProduct(toTile, SteeringDir) * 0.9f;
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
        SteeringFlipTimer = 0.2f; // window deteksi
    }

    SteeringFlipTimer -= GetFrameTime();
    if (SteeringFlipTimer <= 0.f)
        SteeringFlipCount = 0; // reset kalau udah lama gak flip

    return SteeringDir;
}