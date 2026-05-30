#include "seedmanager.h"
#include <nlohmann/json.hpp>

SeedManager g_SeedManager;
#include <fstream>
#include <random>
#include <filesystem>

void SeedManager::InitRun()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist;

    for (int i = 0; i < SEED_COUNT; i++)
        seeds[i] = dist(gen);

    currentStage = 0;
    isRunActive = true;
}

uint32_t SeedManager::GetSeed(int stage) const
{
    if (stage < 0 || stage >= SEED_COUNT)
        return 0;
    return seeds[stage];
}

void SeedManager::SaveMeta(const std::string &filePath)
{
    nlohmann::json j;
    j["seeds"] = nlohmann::json::array();
    for (int i = 0; i < SEED_COUNT; i++)
        j["seeds"].push_back(seeds[i]);
    j["currentStage"] = currentStage;

    std::ofstream file(filePath);
    if (file.is_open())
        file << j.dump(4);
}

bool SeedManager::LoadMeta(const std::string &filePath)
{
    namespace fs = std::filesystem;
    if (!fs::exists(filePath))
        return false;

    std::ifstream file(filePath);
    if (!file.is_open())
        return false;

    nlohmann::json j;
    try
    {
        file >> j;
        auto seedsArr = j["seeds"];
        for (int i = 0; i < SEED_COUNT && i < (int)seedsArr.size(); i++)
            seeds[i] = seedsArr[i];
        currentStage = j.value("currentStage", 0);
        isRunActive = true;
        return true;
    }
    catch (...)
    {
        return false;
    }
}
