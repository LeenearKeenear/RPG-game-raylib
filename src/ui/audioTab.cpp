/**
 * @file audioTab.cpp
 * @brief Implementasi Audio Settings Tab
 * 
 * Handle rendering untuk pengaturan audio.
 */

#include "audioTab.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/json/include/nlohmann/json.hpp"
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

void DrawAudioTab(
    Vector2 mousePosition,
    int startX,
    int startY)
{
    (void)mousePosition;
    int contentStartY = startY + 100;
    const int fontSize = 24;
    int labelX = startX + 40;
    int valueX = startX + 250;

    DrawText("Master Volume", labelX, contentStartY + 15, fontSize, WHITE);
    DrawText("100%", valueX, contentStartY + 15, fontSize, YELLOW);

    DrawText("Music Volume", labelX, contentStartY + 75, fontSize, WHITE);
    DrawText("80%", valueX, contentStartY + 75, fontSize, YELLOW);

    DrawText("SFX Volume", labelX, contentStartY + 135, fontSize, WHITE);
    DrawText("100%", valueX, contentStartY + 135, fontSize, YELLOW);

    DrawText("(Coming Soon)", labelX, contentStartY + 180, 18, GRAY);
}

bool LoadAudioSettings()
{
    if (!std::filesystem::exists(AUDIO_SETTINGS_PATH))
    {
        TraceLog(LOG_INFO, "AUDIO: Tidak ada file settings di %s, pakai default", AUDIO_SETTINGS_PATH);
        return false;
    }

    try
    {
        std::ifstream file(AUDIO_SETTINGS_PATH);
        json root = json::parse(file);

        if (!root.contains("version") || root.at("version").get<int>() != 1)
        {
            TraceLog(LOG_WARNING, "AUDIO: Version mismatch, pakai default");
            return false;
        }

        TraceLog(LOG_INFO, "AUDIO: Settings dimuat dari %s", AUDIO_SETTINGS_PATH);
        return true;
    }
    catch (const std::exception& e)
    {
        TraceLog(LOG_WARNING, "AUDIO: Gagal muat %s: %s", AUDIO_SETTINGS_PATH, e.what());
        return false;
    }
}

bool SaveAudioSettings(int masterVolume, int musicVolume, int sfxVolume)
{
    try
    {
        json root;
        root["version"]      = 1;
        root["masterVolume"] = masterVolume;
        root["musicVolume"]  = musicVolume;
        root["sfxVolume"]    = sfxVolume;

        std::filesystem::path fsPath(AUDIO_SETTINGS_PATH);
        std::filesystem::create_directories(fsPath.parent_path());

        std::string tmpPath = std::string(AUDIO_SETTINGS_PATH) + ".tmp";
        if (std::filesystem::exists(tmpPath))
            std::filesystem::remove(tmpPath);

        {
            std::ofstream file(tmpPath);
            file << root.dump(2);
        }

        std::filesystem::rename(tmpPath, AUDIO_SETTINGS_PATH);
        TraceLog(LOG_INFO, "AUDIO: Settings disimpan ke %s", AUDIO_SETTINGS_PATH);
        return true;
    }
    catch (const std::exception& e)
    {
        TraceLog(LOG_WARNING, "AUDIO: Gagal simpan: %s", e.what());
        return false;
    }
}
