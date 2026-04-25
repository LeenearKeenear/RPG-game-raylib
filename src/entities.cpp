#include "../include/entities.h"
#include <vector>
#include <algorithm>
#include <set>
#include <string>

namespace Entities
{
    static std::vector<Entity *> Registry;
    static std::vector<Entity *> DynamicRegistry;
    static std::set<std::string> DeadEntities;

    void Init()
    {
        Registry.clear();
        DynamicRegistry.clear();
    }

    void Update()
    {
        for (size_t i = 0; i < Registry.size(); i++)
        {
            if (Registry[i] && Registry[i]->IsActive)
            {
                Registry[i]->Update();
            }
        }
    }

    void Render()
    {
        // Y-axis priority (Depth sorting)
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

    void RegisterDeath(const std::string& mapPath, int objectId)
    {
        DeadEntities.insert(mapPath + "_" + std::to_string(objectId));
    }

    bool IsAlreadyDead(const std::string& mapPath, int objectId)
    {
        return DeadEntities.find(mapPath + "_" + std::to_string(objectId)) != DeadEntities.end();
    }
}
