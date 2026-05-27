#include "worldgenenartion.h"
#include <ctime>
#include <queue>

static uint64_t currentRunSeed = 0;

// Direction lookup: index 0-3 = UP, DOWN, LEFT, RIGHT
static const int DIR_DR[] = {-1, 1, 0, 0};
static const int DIR_DC[] = {0, 0, -1, 1};
static const int DIR_EXIT_MASK[] = {EXIT_NORTH, EXIT_SOUTH, EXIT_WEST, EXIT_EAST};
static const int DIR_OPPOSITE_MASK[] = {EXIT_SOUTH, EXIT_NORTH, EXIT_EAST, EXIT_WEST};

uint64_t GenerateRunSeed(void)
{
    currentRunSeed = static_cast<uint64_t>(time(nullptr));
    return currentRunSeed;
}

uint64_t GetCurrentRunSeed(void)
{
    return currentRunSeed;
}

void WorldGenLayout::InitGrid()
{
    grid.assign(WG_GRID_SIZE, std::vector<WorldCell>(WG_GRID_SIZE, {CELL_EMPTY, EXIT_NONE, nullptr}));
}

WorldGenLayout::WorldGenLayout(uint64_t seed)
    : wgRng(seed)
{
    InitGrid();
}

const std::vector<std::vector<WorldCell>> &WorldGenLayout::GetGrid() const
{
    return grid;
}

int WorldGenCanvas::RotateExitMask(int mask, int degrees)
{
    int steps = (degrees / 90) % 4;
    for (int i = 0; i < steps; i++)
    {
        int newMask = 0;
        if (mask & EXIT_NORTH)
            newMask |= EXIT_EAST;
        if (mask & EXIT_EAST)
            newMask |= EXIT_SOUTH;
        if (mask & EXIT_SOUTH)
            newMask |= EXIT_WEST;
        if (mask & EXIT_WEST)
            newMask |= EXIT_NORTH;
        mask = newMask;
    }
    return mask;
}

bool WorldGenLayout::IsLeaf(int r, int c) const
{
    int mask = grid[r][c].exitMask;
    int count = 0;
    if (mask & EXIT_NORTH)
        count++;
    if (mask & EXIT_SOUTH)
        count++;
    if (mask & EXIT_EAST)
        count++;
    if (mask & EXIT_WEST)
        count++;
    return count == 1;
}

bool WorldGenLayout::IsAdjacentToType(int r, int c, CellType type) const
{
    for (int d = 0; d < NUM_DIRS; d++)
    {
        if (!(grid[r][c].exitMask & DIR_EXIT_MASK[d]))
            continue;
        int nr = r + DIR_DR[d];
        int nc = c + DIR_DC[d];

        if (nr < 0 || nr >= WG_GRID_SIZE || nc < 0 || nc >= WG_GRID_SIZE)
            continue;

        if (grid[nr][nc].type == type)
            return true;
    }
    return false;
}

int WorldGenLayout::CountActiveCells() const
{
    int count = 0;
    for (int r = 0; r < WG_GRID_SIZE; r++)
        for (int c = 0; c < WG_GRID_SIZE; c++)
            if (grid[r][c].type != CELL_EMPTY)
                count++;
    return count;
}

std::vector<std::vector<int>> WorldGenLayout::ComputeDepth() const
{
    std::vector<std::vector<int>> depth(WG_GRID_SIZE, std::vector<int>(WG_GRID_SIZE, DEPTH_UNVISITED));

    // Cari START
    int startR = INVALID_INDEX, startC = INVALID_INDEX;
    for (int r = 0; r < WG_GRID_SIZE; r++)
        for (int c = 0; c < WG_GRID_SIZE; c++)
            if (grid[r][c].type == CELL_START)
            {
                startR = r;
                startC = c;
            }

    if (startR == INVALID_INDEX)
        return depth;

    // DFS iteratif
    std::queue<std::pair<int, int>> q;
    q.push({startR, startC});
    depth[startR][startC] = 0;

    while (!q.empty())
    {
        auto [r, c] = q.front();
        q.pop();

        for (int d = 0; d < NUM_DIRS; d++)
        {
            if (!(grid[r][c].exitMask & DIR_EXIT_MASK[d]))
                continue;
            int nr = r + DIR_DR[d];
            int nc = c + DIR_DC[d];
            if (depth[nr][nc] != DEPTH_UNVISITED)
                continue;
            depth[nr][nc] = depth[r][c] + 1;
            q.push({nr, nc});
        }
    }

    return depth;
}

void WorldGenLayout::RunPrims()
{
    // 1. Pilih random edge cell buat START
    // Edge cells: row 0, row 3, col 0, col 3
    std::vector<std::pair<int, int>> edgeCells;
    for (int r = 0; r < WG_GRID_SIZE; r++)
        for (int c = 0; c < WG_GRID_SIZE; c++)
            if (r == 0 || r == WG_GRID_SIZE - 1 || c == 0 || c == WG_GRID_SIZE - 1)
                edgeCells.push_back({r, c});

    std::uniform_int_distribution<int> dist(0, edgeCells.size() - 1);
    auto [startR, startC] = edgeCells[dist(wgRng)];
    grid[startR][startC].type = CELL_START;

    // 2. Prim's — frontier list
    std::vector<std::pair<int, int>> frontier;
    auto addFrontier = [&](int r, int c)
    {
        if (r < 0 || r >= WG_GRID_SIZE || c < 0 || c >= WG_GRID_SIZE)
            return;
        if (grid[r][c].type != CELL_EMPTY)
            return;
        if (std::find(frontier.begin(), frontier.end(), std::make_pair(r, c)) != frontier.end())
            return;
        frontier.push_back({r, c});
    };

    addFrontier(startR - 1, startC);
    addFrontier(startR + 1, startC);
    addFrontier(startR, startC - 1);
    addFrontier(startR, startC + 1);

    int activeCells = WG_PRIM_START_CELLS;

    while (!frontier.empty() && activeCells < WG_PRIM_MAX_CELLS)
    {
        // Pilih random dari frontier
        std::uniform_int_distribution<int> fd(0, frontier.size() - 1);
        int idx = fd(wgRng);
        auto [r, c] = frontier[idx];
        frontier.erase(frontier.begin() + idx);

        // Aktifkan cell
        grid[r][c].type = CELL_ENEMY;
        activeCells++;

        // Set exitMask dua arah ke semua neighbor aktif
        for (int d = 0; d < NUM_DIRS; d++)
        {
            int nr = r + DIR_DR[d];
            int nc = c + DIR_DC[d];
            if (nr < 0 || nr >= WG_GRID_SIZE || nc < 0 || nc >= WG_GRID_SIZE)
                continue;
            if (grid[nr][nc].type == CELL_EMPTY)
                continue;

            grid[r][c].exitMask |= DIR_EXIT_MASK[d]; // exit dari cell baru ke neighbor
            grid[nr][nc].exitMask |= DIR_OPPOSITE_MASK[d];
        }

        // Tambah neighbors ke frontier
        for (int d = 0; d < NUM_DIRS; d++)
            addFrontier(r + DIR_DR[d], c + DIR_DC[d]);

        // Random stop setelah WG_PRIM_MIN_CELLS
        if (activeCells >= WG_PRIM_MIN_CELLS)
        {
            std::uniform_int_distribution<int> stopRoll(0, PERCENT_MAX);
            if (stopRoll(wgRng) < WG_PRIM_STOP_CHANCE)
                break;
        }
    }

    // 3. Cari cell paling jauh dari start → CELL_FINISH
    auto depth = ComputeDepth();
    int maxDepth = INVALID_INDEX;
    int finishR = startR, finishC = startC;
    for (int r = 0; r < WG_GRID_SIZE; r++)
        for (int c = 0; c < WG_GRID_SIZE; c++)
        {
            if (grid[r][c].type == CELL_EMPTY)
                continue;
            if (r == startR && c == startC)
                continue;
            if (depth[r][c] > maxDepth)
            {
                maxDepth = depth[r][c];
                finishR = r;
                finishC = c;
            }
        }
    grid[finishR][finishC].type = CELL_FINISH;
}

void WorldGenLayout::AssignCellTypes()
{
    auto depth = ComputeDepth();
    int total = CountActiveCells();

    // Kumpulin leaf candidates (bukan START/FINISH, gak adjacent ke START/FINISH)
    std::vector<std::pair<int, int>> leaves;
    for (int r = 0; r < WG_GRID_SIZE; r++)
        for (int c = 0; c < WG_GRID_SIZE; c++)
        {
            if (grid[r][c].type == CELL_EMPTY)
                continue;
            if (grid[r][c].type == CELL_START)
                continue;
            if (grid[r][c].type == CELL_FINISH)
                continue;
            if (IsAdjacentToType(r, c, CELL_START))
                continue;
            if (IsAdjacentToType(r, c, CELL_FINISH))
                continue;
            if (IsLeaf(r, c))
                leaves.push_back({r, c});
        }

    // Fallback 1: semua non-START/FINISH yang memenuhi constraint
    if (leaves.empty())
    {
        for (int r = 0; r < WG_GRID_SIZE; r++)
            for (int c = 0; c < WG_GRID_SIZE; c++)
            {
                if (grid[r][c].type == CELL_EMPTY)
                    continue;
                if (grid[r][c].type == CELL_START)
                    continue;
                if (grid[r][c].type == CELL_FINISH)
                    continue;
                if (IsAdjacentToType(r, c, CELL_START))
                    continue;
                if (IsAdjacentToType(r, c, CELL_FINISH))
                    continue;
                leaves.push_back({r, c});
            }
    }

    // Fallback 2: force assign by depth
    if (leaves.empty())
    {
        int maxDepth = INVALID_INDEX;
        std::pair<int, int> best = {INVALID_INDEX, INVALID_INDEX};
        for (int r = 0; r < WG_GRID_SIZE; r++)
            for (int c = 0; c < WG_GRID_SIZE; c++)
            {
                if (grid[r][c].type == CELL_EMPTY)
                    continue;
                if (grid[r][c].type == CELL_START)
                    continue;
                if (grid[r][c].type == CELL_FINISH)
                    continue;
                if (IsAdjacentToType(r, c, CELL_START))
                    continue;
                if (IsAdjacentToType(r, c, CELL_FINISH))
                    continue;
                if (depth[r][c] > maxDepth)
                {
                    maxDepth = depth[r][c];
                    best = {r, c};
                }
            }
        if (best.first != INVALID_INDEX)
            leaves.push_back(best);
    }

    std::shuffle(leaves.begin(), leaves.end(), wgRng);
    std::uniform_int_distribution<int> chance(0, PERCENT_MAX);
    int leafIdx = 0;

    // 1. Treasure wajib 1
    if (leafIdx < (int)leaves.size())
    {
        auto [r, c] = leaves[leafIdx++];
        grid[r][c].type = CELL_TREASURE;
    }

    // 2. Treasure ke-2, chance 20% kalau total > 7
    int secondTreasureThreshold = 7;
    int secondTreasureChance = 20;
    if (total > secondTreasureThreshold && leafIdx < (int)leaves.size() && chance(wgRng) < secondTreasureChance)
    {
        auto [r, c] = leaves[leafIdx++];
        grid[r][c].type = CELL_TREASURE;
    }

    // 3. Trader, chance 30%
    int traderChance = 30;
    if (leafIdx < (int)leaves.size() && chance(wgRng) < traderChance)
    {
        auto [r, c] = leaves[leafIdx++];
        grid[r][c].type = CELL_TRADER;
    }

    // 4. Elite, max 2, kalau total > 6
    int finishDepth = 0;
    for (int r = 0; r < WG_GRID_SIZE; r++)
        for (int c = 0; c < WG_GRID_SIZE; c++)
            if (grid[r][c].type == CELL_FINISH)
                finishDepth = depth[r][c];

    int eliteThreshold = 6;
    int eliteCount = 0;
    int maxEliteCount = 2;
    if (total > eliteThreshold)
    {
        int eliteDepthHalf = finishDepth / 2;
        for (int i = leafIdx; i < (int)leaves.size() && eliteCount < maxEliteCount; i++)
        {
            auto [r, c] = leaves[i];
            if (depth[r][c] > eliteDepthHalf && depth[r][c] < finishDepth)
            {
                grid[r][c].type = CELL_ENEMY_ELITE;
                leaves.erase(leaves.begin() + i);
                i--;
                eliteCount++;
                leafIdx++;
            }
        }
    }

    // 5. Special, chance 20%
    int specialChance = 20;
    if (leafIdx < (int)leaves.size() && chance(wgRng) < specialChance)
    {
        auto [r, c] = leaves[leafIdx++];
        grid[r][c].type = CELL_SPECIAL;
    }
    TraceLog(LOG_INFO, "Total active: %d, Leaves found: %d", total, (int)leaves.size());
}

void WorldGenLayout::DebugPrintGrid() const
{
    for (int r = 0; r < WG_GRID_SIZE; r++)
    {
        std::string row = "";
        for (int c = 0; c < WG_GRID_SIZE; c++)
        {
            switch (grid[r][c].type)
            {
            case CELL_EMPTY:
                row += "[  ] ";
                break;
            case CELL_START:
                row += "[ST] ";
                break;
            case CELL_ENEMY:
                row += "[EN] ";
                break;
            case CELL_ENEMY_ELITE:
                row += "[EL] ";
                break;
            case CELL_TREASURE:
                row += "[TR] ";
                break;
            case CELL_TRADER:
                row += "[TD] ";
                break;
            case CELL_FINISH:
                row += "[FN] ";
                break;
            case CELL_BOSS:
                row += "[BS] ";
                break;
            case CELL_SPECIAL:
                row += "[SP] ";
                break;
            }
        }
        TraceLog(LOG_INFO, "%s", row.c_str());
    }

    // Print exitMask per cell
    for (int r = 0; r < WG_GRID_SIZE; r++)
    {
        std::string row = "";
        for (int c = 0; c < WG_GRID_SIZE; c++)
        {
            int mask = grid[r][c].exitMask;
            std::string exits = "";
            if (mask & EXIT_NORTH)
                exits += "N";
            if (mask & EXIT_SOUTH)
                exits += "S";
            if (mask & EXIT_EAST)
                exits += "E";
            if (mask & EXIT_WEST)
                exits += "W";
            if (exits.empty())
                exits = "--";
            row += "[" + exits + "] ";
        }
        TraceLog(LOG_INFO, "%s", row.c_str());
    }
}

WorldGenPrefab::WorldGenPrefab(TilesonMapData *existingData)
    : data(existingData)
{
}

WorldGenPrefab WorldGenCanvas::ResolveRotation(CellType type, int exitMask, std::mt19937_64 &rng)
{
    RoomPool &roomPool = pools->GetPoolForType(type);
    int baseMask = 0;
    WeightedPool *pool = SelectPool(type, exitMask, roomPool, baseMask);
    if (!pool || pool->prefabs.empty())
        return WorldGenPrefab();

    std::uniform_int_distribution<int> dist(0, (int)pool->prefabs.size() - 1);
    TilesonMapData *rawPrefab = pool->prefabs[dist(rng)];

    WorldGenPrefab wrapper(rawPrefab); // non-owning, pool masih punya data

    for (int deg : {0, 90, 180, 270})
    {
        if (RotateExitMask(baseMask, deg) == exitMask)
            return wrapper.Rotate(deg, deg); // owned copy hasil rotate
    }

    return wrapper.Rotate(0, 0); // fallback — owned copy tanpa rotasi
}

WeightedPool *WorldGenCanvas::SelectPool(CellType type, int exitMask, RoomPool &roomPool, int &outBaseMask)
{
    int exitCount = __builtin_popcount(exitMask);

    if (type == CELL_START || type == CELL_FINISH || type == CELL_BOSS || type == CELL_TRADER)
    {
        outBaseMask = EXIT_NORTH;
        return &roomPool.u;
    }

    if (type == CELL_TREASURE || type == CELL_ENEMY_ELITE)
    {
        if (exitCount >= 2)
        {
            outBaseMask = EXIT_NORTH | EXIT_EAST;
            return &roomPool.ur;
        }
        else
        {
            outBaseMask = EXIT_NORTH;
            return &roomPool.u;
        }
    }

    // ENEMY + SPECIAL
    if (exitCount == 1)
    {
        outBaseMask = EXIT_NORTH;
        return &roomPool.u;
    }
    if (exitCount == 2)
    {
        bool straight = ((exitMask & EXIT_NORTH) && (exitMask & EXIT_SOUTH)) ||
                        ((exitMask & EXIT_EAST) && (exitMask & EXIT_WEST));
        if (straight)
        {
            outBaseMask = EXIT_NORTH | EXIT_SOUTH;
            return &roomPool.ud;
        }
        else
        {
            outBaseMask = EXIT_NORTH | EXIT_EAST;
            return &roomPool.ur;
        }
    }
    if (exitCount == 3)
    {
        outBaseMask = EXIT_NORTH | EXIT_SOUTH | EXIT_EAST;
        return &roomPool.udr;
    }
    outBaseMask = EXIT_NORTH | EXIT_SOUTH | EXIT_EAST | EXIT_WEST;
    return &roomPool.udrl;
}

void WorldGenLayout::Generate()
{
    InitGrid();
    RunPrims();
    AssignCellTypes();
    DebugPrintGrid();
}