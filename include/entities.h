#pragma once
#include <vector>
#include <string>
#include "entity.h"

namespace Entities
{
    void Init();
    void Update();
    void Render();
    void Shutdown();
    void Add(Entity *entity);
    void AddDynamic(Entity *entity);
    void Clear();
    const std::vector<Entity *> &GetRegistry();

    void RegisterDeath(const std::string& mapPath, int objectId);
    bool IsAlreadyDead(const std::string& mapPath, int objectId);
}