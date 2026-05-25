#include "worldgenenartion.h"

void WorldGenLayout::InitGrid()
{
    grid.assign(WG_GRID_SIZE, std::vector<WorldCell>(WG_GRID_SIZE, {CELL_EMPTY, EXIT_NONE, nullptr}));
}

WorldGenLayout::WorldGenLayout(uint64_t seed)
    : worldSeed(seed), wgRng(seed)
{
    InitGrid();
}

const std::vector<std::vector<WorldCell>> &WorldGenLayout::GetGrid() const
{
    return grid;
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

    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    addFrontier(startR - 1, startC);
    addFrontier(startR + 1, startC);
    addFrontier(startR, startC - 1);
    addFrontier(startR, startC + 1);

    int activeCells = 1;
    constexpr int MIN_CELLS = 6;
    constexpr int MAX_CELLS = 8;

    while (!frontier.empty() && activeCells < MAX_CELLS)
    {
        // Pilih random dari frontier
        std::uniform_int_distribution<int> fd(0, frontier.size() - 1);
        int idx = fd(wgRng);
        auto [r, c] = frontier[idx];
        frontier.erase(frontier.begin() + idx);

        // Aktifkan cell
        grid[r][c].type = CELL_ENEMY; // placeholder, nanti di-assign proper
        activeCells++;

        // Tambah neighbors ke frontier
        for (int d = 0; d < 4; d++)
            addFrontier(r + dr[d], c + dc[d]);
    }

    // 3. Cari cell paling jauh dari start → CELL_FINISH
    int maxDist = -1;
    int finishR = startR, finishC = startC;
    for (int r = 0; r < WG_GRID_SIZE; r++)
        for (int c = 0; c < WG_GRID_SIZE; c++)
        {
            if (grid[r][c].type == CELL_EMPTY)
                continue;
            if (r == startR && c == startC)
                continue;
            int dist = abs(r - startR) + abs(c - startC);
            if (dist > maxDist)
            {
                maxDist = dist;
                finishR = r;
                finishC = c;
            }
        }
    grid[finishR][finishC].type = CELL_FINISH;
}

void WorldGenLayout::AssignCellTypes()
{
    // skip for visual test
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
                case CELL_EMPTY:        row += "[  ] "; break;
                case CELL_START:        row += "[ST] "; break;
                case CELL_ENEMY:        row += "[EN] "; break;
                case CELL_ENEMY_ELITE:  row += "[EL] "; break;
                case CELL_TREASURE:     row += "[TR] "; break;
                case CELL_TRADER:       row += "[TD] "; break;
                case CELL_FINISH:       row += "[FN] "; break;
                case CELL_BOSS:         row += "[BS] "; break;
                case CELL_SPECIAL:      row += "[SP] "; break;
            }
        }
        TraceLog(LOG_INFO, "%s", row.c_str());
    }
}

void WorldGenLayout::Generate()
{
    InitGrid();
    RunPrims();
    DebugPrintGrid();
    AssignCellTypes();
}