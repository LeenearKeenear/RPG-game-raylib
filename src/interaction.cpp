#include "../include/interaction.h"
#include "../include/player.h"
#include "../include/input.h"
#include "../include/mapLogic.h"
#include "../include/map.h"
#include "../include/screen.h"
#include "../lib/raylib/include/raymath.h"

namespace Interaction
{
    void HandleInteractions(Player &player)
    {
        UpdateRaycast(player);
        CheckDoors(player);
        CheckProps(player);

        if (player.pendingGoBack)
        {
            player.pendingGoBack = false;
            GoBack();
        }

        if (player.pendingSwitchMap)
        {
            player.pendingSwitchMap = false;
            SwitchMap(player.pendingMapPath.c_str(), player.pendingDoorName.c_str());
        }
    }

    void UpdateRaycast(Player &player)
    {
        Vector2 playerCenter = {
            player.Position.x + player.HitboxOffsetX + player.HitboxWidth / 2,
            player.Position.y + player.HitboxOffsetY + player.HitboxHeight / 2};

        Vector2 mouseWorld = GetScreenToWorld2D(GetVirtualMousePosition(player.State), camera);
        Vector2 aimDir = Vector2Normalize(Vector2Subtract(mouseWorld, playerCenter));

        std::vector<MapObject> propObjects;
        for (auto &obj : tilesonMap->Objects)
        {
            if (obj.layerName == OBJECT_LAYER_NAME)
                propObjects.push_back(obj);
        }

        player.LastHit = player.Ray.Cast(playerCenter, aimDir, player.INTERACT_RANGE, propObjects);
    }

    void CheckDoors(Player &player)
    {
        Rectangle playerHitbox = BuildHitbox(player.Position, player.HitboxOffsetX, player.HitboxOffsetY, player.HitboxWidth, player.HitboxHeight);
        std::vector<MapObject *> doors = TiledHelperFunction.GetObjectsByType(DOOR_TYPE_OBJECT_NAME);

        for (const auto &door : doors)
        {
            if (!CheckCollisionRecs(playerHitbox, door->bounds))
                continue;
            if (!InputInstance.IsInteract())
                continue;

            auto mapIt = door->properties.find("target_map");
            auto doorIt = door->properties.find("target_door");

            if (mapIt != door->properties.end() && doorIt != door->properties.end())
            {
                player.pendingSwitchMap = true;
                player.pendingMapPath = mapIt->second.getValue<std::string>();
                player.pendingDoorName = doorIt->second.getValue<std::string>();
                return;
            }
        }
    }

    void CheckProps(Player &player)
    {
        if (!player.LastHit.hit)
            return;
        if (!InputInstance.IsInteract())
            return;

        const std::string &type = player.LastHit.object->type;
        TraceLog(LOG_INFO, "Interacting with: '%s' (type: %s)", player.LastHit.object->name.c_str(), type.c_str());

        if (type == CHEST_TYPE_OBJECT_NAME)
        {
            TraceLog(LOG_INFO, "Opening chest: '%s'", player.LastHit.object->name.c_str());
        }
    }
}
