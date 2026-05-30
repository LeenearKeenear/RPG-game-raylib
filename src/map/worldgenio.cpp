#include "worldgenio.h"
#include "seedmanager.h"
#include "map.h"
#include "propsbehavior.h"
#include "entities.h"
#include "item.h"
#include "screen.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

static const char *WORLDSEED_DIR = "assets/maps/World_generation/worldseed";
static const char *SAVE_DATA_DIR = "assets/maps/World_generation/worldseed/save_data";
static const char *META_FILE = "assets/maps/World_generation/worldseed/save_meta_data.json";
static const char *RUNTIME_FILE = "assets/maps/World_generation/worldseed/save_runtime_state.json";
static const char *BG_MAP = "assets/maps/World_generation/background_map.json";

namespace WorldgenIO
{

bool InitRun()
{
    g_SeedManager.InitRun();

    fs::create_directories(SAVE_DATA_DIR);

    for (int i = 0; i < SeedManager::SEED_COUNT; i++)
    {
        char dst[256];
        snprintf(dst, sizeof(dst), "%s/background_map_stage_%d.json", SAVE_DATA_DIR, i + 1);
        fs::copy_file(BG_MAP, dst, fs::copy_options::overwrite_existing);

        // Fix relative texture paths — file jadi 2 folder lebih dalam dari asli
        std::ifstream in(dst);
        if (in.is_open())
        {
            std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            in.close();

            // Cari posisi "image":"../../textures/" dan ganti jadi "../../../textures/"
            size_t pos = 0;
            const std::string oldPrefix = "\"../../textures/";
            const std::string newPrefix = "\"../../../textures/";
            while ((pos = content.find(oldPrefix, pos)) != std::string::npos)
            {
                content.replace(pos, oldPrefix.length(), newPrefix);
                pos += newPrefix.length();
            }

            std::ofstream out(dst);
            if (out.is_open())
                out << content;
        }
    }

    g_SeedManager.SaveMeta(META_FILE);

    nlohmann::json empty;
    std::ofstream rt(RUNTIME_FILE);
    if (rt.is_open())
        rt << empty.dump(4);

    return true;
}

bool SaveRuntimeState(int stageIndex)
{
    nlohmann::json root;
    if (fs::exists(RUNTIME_FILE))
    {
        std::ifstream in(RUNTIME_FILE);
        if (in.is_open())
        {
            try { in >> root; }
            catch (...) { root = nlohmann::json::object(); }
        }
    }

    std::string key = "stage_" + std::to_string(stageIndex);
    nlohmann::json &stageData = root[key];

    stageData["chests"] = nlohmann::json::array();
    for (auto &pos : chestManager.GetConsumedPositions())
        stageData["chests"].push_back(pos);

    stageData["crates"] = nlohmann::json::array();
    for (auto &pos : crateManager.GetConsumedPositions())
        stageData["crates"].push_back(pos);

    stageData["bombs"] = nlohmann::json::array();
    for (auto &pos : bombManager.GetConsumedPositions())
        stageData["bombs"].push_back(pos);

    stageData["deadEnemies"] = nlohmann::json::array();
    for (auto &entry : Entities::GetDeadEntries())
        stageData["deadEnemies"].push_back(entry);

    stageData["itemDrops"] = nlohmann::json::array();
    for (auto &item : itemData.activeItems)
    {
        nlohmann::json itemJson;
        itemJson["defId"] = item.definitionId;
        itemJson["amount"] = item.amount;
        itemJson["x"] = item.position.x;
        itemJson["y"] = item.position.y;
        stageData["itemDrops"].push_back(itemJson);
    }

    std::ofstream out(RUNTIME_FILE);
    if (!out.is_open())
        return false;
    out << root.dump(4);
    return true;
}

bool LoadRuntimeState(int stageIndex)
{
    if (!fs::exists(RUNTIME_FILE))
        return false;

    std::ifstream in(RUNTIME_FILE);
    if (!in.is_open())
        return false;

    nlohmann::json root;
    try { in >> root; }
    catch (...) { return false; }

    std::string key = "stage_" + std::to_string(stageIndex);
    if (!root.contains(key))
        return false;

    nlohmann::json &stageData = root[key];

    if (stageData.contains("chests"))
    {
        std::unordered_set<std::string> positions;
        for (auto &pos : stageData["chests"])
            positions.insert(pos.get<std::string>());
        chestManager.SetConsumedPositions(positions);
    }

    if (stageData.contains("crates"))
    {
        std::unordered_set<std::string> positions;
        for (auto &pos : stageData["crates"])
            positions.insert(pos.get<std::string>());
        crateManager.SetConsumedPositions(positions);
    }

    if (stageData.contains("bombs"))
    {
        std::unordered_set<std::string> positions;
        for (auto &pos : stageData["bombs"])
            positions.insert(pos.get<std::string>());
        bombManager.SetConsumedPositions(positions);
    }

    if (stageData.contains("deadEnemies"))
    {
        std::set<std::string> entries;
        for (auto &entry : stageData["deadEnemies"])
            entries.insert(entry.get<std::string>());
        Entities::SetDeadEntries(entries);
    }

    if (stageData.contains("itemDrops"))
    {
        itemData.activeItems.clear();
        for (auto &itemJson : stageData["itemDrops"])
        {
            ItemSpawn item;
            item.definitionId = itemJson["defId"];
            item.amount = itemJson["amount"];
            item.position.x = itemJson["x"];
            item.position.y = itemJson["y"];
            itemData.activeItems.push_back(item);
        }
    }

    return true;
}

std::string GetStagePath(int stageIndex)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "%s/background_map_stage_%d.json", SAVE_DATA_DIR, stageIndex + 1);
    return std::string(buf);
}

void NextStage()
{
    int oldStage = g_SeedManager.GetCurrentStage();
    SaveRuntimeState(oldStage);

    TraceLog(LOG_INFO, "NextStage: %d -> %d", oldStage + 1, oldStage + 2);

    if (oldStage >= SeedManager::SEED_COUNT - 1)
    {
        // Sudah stage terakhir (boss) — balik lobby, reset run
        g_SeedManager.ResetRun();
        gState->currentScreen = MAIN_MENU;
        return;
    }

    g_SeedManager.NextStage();
    g_SeedManager.SaveMeta(META_FILE);

    std::string stagePath = GetStagePath(g_SeedManager.GetCurrentStage());
    SwitchMap(stagePath.c_str(), "start");

    // Stage baru di-load — trim stack biar cuman nyimpen 1 prev stage
    TrimStageStack();
}

void PrevStage()
{
    if (!g_SeedManager.CanGoBack())
        return;

    int oldStage = g_SeedManager.GetCurrentStage();
    SaveRuntimeState(oldStage);

    int targetStage = g_SeedManager.GoBackStage();
    TraceLog(LOG_INFO, "PrevStage: %d -> %d", oldStage + 1, targetStage + 1);

    g_SeedManager.SaveMeta(META_FILE);

    std::string stagePath = GetStagePath(targetStage);
    SwitchMap(stagePath.c_str(), "finish");

    TrimStageStack();
}

} // namespace WorldgenIO
