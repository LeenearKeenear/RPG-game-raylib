#include "../include/entities.h"
#include "../include/player.h"
#include "../include/enemy.h"
#include "../include/mapLogic.h"
#include "../include/map.h"
#include "../include/tiles.h"
#include <vector>

namespace Entities {

    static std::vector<Enemy*> enemies;

    void Init() {
        LoadTileTexture(TEXTURE_ENEMY, "texture/enemies.png");
    }

    void SpawnEnemies() {
        for (auto e : enemies) delete e;
        enemies.clear();

        if (tilesonMap == nullptr) return;

        std::vector<MapObject*> spawnPoints = TiledHelperFunction.GetObjectsByType("spawn");
        for (auto obj : spawnPoints) {
            if (obj->name == "spawn_enemy") {
                Vector2 pos = { obj->bounds.x, obj->bounds.y };
                
                const AnimationSet* animSet = &SlimeAnimationSet;
                std::string enemyName = "Slime";

                auto it = obj->properties.find("enemy_type");
                if (it != obj->properties.end()) {
                    std::string type = it->second.getValue<std::string>();
                    if (type == "skeleton") {
                        animSet = &SkeletonAnimationSet;
                        enemyName = "Skeleton";
                    } else if (type == "wolf") {
                        animSet = &WolfAnimationSet;
                        enemyName = "Wolf";
                    }
                }

                enemies.push_back(new Enemy(pos, enemyName, *animSet));
                TraceLog(LOG_INFO, "Entities: Spawned %s at (%.1f, %.1f)", enemyName.c_str(), pos.x, pos.y);
            }
        }
    }

    void Update() {
        PlayerInstance.Tick();

        float dt = GetFrameTime();
        for (auto e : enemies) {
            e->Update(dt);
        }
    }

    void Render() {
        PlayerInstance.Render();

        for (auto e : enemies) {
            e->Render();
        }
    }

    void Shutdown() {
        for (auto e : enemies) delete e;
        enemies.clear();
    }
}