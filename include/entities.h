#pragma once
#include <vector>
#include "entity.h"

namespace Entities
{
    void Init();
    void Update();
    void Render();
    void Shutdown();
    void Add(Entity *entity);
    void Clear();
}