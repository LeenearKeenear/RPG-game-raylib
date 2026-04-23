#include "../include/entities.h"
#include <vector>

namespace Entities
{
    static std::vector<Entity *> Registry;

    void Init()
    {
        Registry.clear();
    }

    void Update()
    {
        for (auto entity : Registry)
        {
            if (entity && entity->IsActive)
            {
                entity->Update();
            }
        }
    }

    void Render()
    {
        for (auto entity : Registry)
        {
            if (entity && entity->IsActive)
            {
                entity->Render();
            }
        }
    }

    void Shutdown()
    {
        Registry.clear();
    }

    void Add(Entity *entity)
    {
        if (entity)
        {
            Registry.push_back(entity);
        }
    }

    void Clear()
    {
        Registry.clear();
    }
}