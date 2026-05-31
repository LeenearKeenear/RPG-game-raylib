#pragma once
#include <string>

#include "../lib/raylib/include/raylib.h"

namespace WorldgenIO
{
    bool InitRun(int saveSlot);
    bool SaveRuntimeState(int stageIndex);
    bool LoadRuntimeState(int stageIndex);
    void NextStage();
    void PrevStage();
    std::string GetStagePath(int stageIndex);
    std::string GetMetaPath(int slot);
    int GetNextAvailableSlot();
    int GetTopSlot();
}
