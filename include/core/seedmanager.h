#pragma once
#include <cstdint>
#include <string>

class SeedManager
{
public:
    static constexpr int SEED_COUNT = 2; // TODO: ganti balik ke 5 setelah testing

    void InitRun(int saveSlot);
    uint32_t GetSeed(int stage) const;
    int GetCurrentStage() const { return currentStage; }
    void SetCurrentStage(int stage) { currentStage = stage; }
    int GetCurrentSlot() const { return currentSlot; }
    void SetCurrentSlot(int slot) { currentSlot = slot; }
    void NextStage() { if (currentStage < SEED_COUNT - 1) { prevStage = currentStage; currentStage++; } }
    bool CanGoBack() const { return prevStage >= 0; }
    int GoBackStage() { int s = prevStage; prevStage = -1; return s; }
    bool IsRunActive() const { return isRunActive; }
    void ResetRun() { isRunActive = false; currentStage = 0; prevStage = -1; }

    void SaveMeta(const std::string &filePath);
    bool LoadMeta(const std::string &filePath);

private:
    uint32_t seeds[SEED_COUNT] = {};
    int currentStage = 0;
    int prevStage = -1;
    int currentSlot = 1;
    bool isRunActive = false;
};

extern SeedManager g_SeedManager;
