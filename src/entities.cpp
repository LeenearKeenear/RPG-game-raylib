#include "../include/entities.h"
#include <vector>
#include <algorithm>


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
        // Y-axis priority (Depth sorting)
        // Sort entities by their Y position so that entities further down are drawn last (on top)
        std::sort(Registry.begin(), Registry.end(), [](Entity *a, Entity *b) {
            if (!a) return false;
            if (!b) return true;
            return a->Position.y < b->Position.y;
        });

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
