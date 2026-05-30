#pragma once
#include <string>

namespace WorldgenIO
{
    bool InitRun();
    bool SaveRuntimeState(int stageIndex);
    bool LoadRuntimeState(int stageIndex);
    void NextStage();
    void PrevStage();
    std::string GetStagePath(int stageIndex);
}
