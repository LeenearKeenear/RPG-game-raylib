#include "../include/entities.h"
#include <vector>

namespace Entities
{
    static std::vector<Entity *> Registry;
    static std::vector<Entity *> DynamicRegistry;

    void Init()
    {
        Registry.clear();
        DynamicRegistry.clear();
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
        for (auto entity : DynamicRegistry)
        {
            delete entity;
        }
        DynamicRegistry.clear();
        Registry.clear();
    }

    void Add(Entity *entity)
    {
        if (entity)
        {
            Registry.push_back(entity);
        }
    }

    void AddDynamic(Entity *entity)
    {
        if (entity)
        {
            DynamicRegistry.push_back(entity);
            Registry.push_back(entity);
        }
    }

    void Clear()
    {
        for (auto entity : DynamicRegistry)
        {
            delete entity;
        }
        DynamicRegistry.clear();
        Registry.clear();
    }

    const std::vector<Entity *> &GetRegistry()
    {
        return Registry;
    }
}