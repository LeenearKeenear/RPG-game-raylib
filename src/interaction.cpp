#include "../include/interaction.h"
#include "../include/player.h"
#include "../include/input.h"
#include "../include/mapLogic.h"
#include "../include/map.h"
#include "../include/screen.h"
#include "../lib/raylib/include/raymath.h"

namespace Interaction
{
    /**
     * Pengendali tingkat atas untuk semua interaksi lingkungan.
     */
    void HandleInteractions(Player &player)
    {
        UpdateRaycast(player);
        CheckDoors(player);    ///< Interaksi berbasis kedekatan (proximity)
        CheckProps(player);    ///< Interaksi berbasis raycast
    }

    /**
     * Melakukan logika perpindahan map secara aktual.
     * Dipisahkan dari pemeriksaan untuk menghindari korupsi status di tengah update.
     */
    void ExecutePendingTransitions(Player &player)
    {
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

    /**
     * Memperbarui raycast interaksi pemain.
     * Memproyeksikan ray pendek ke arah mouse untuk mendeteksi objek yang dapat berinteraksi.
     */
    void UpdateRaycast(Player &player)
    {
        Vector2 playerCenter = {
            player.Position.x + player.HitboxOffsetX + player.HitboxWidth / 2,
            player.Position.y + player.HitboxOffsetY + player.HitboxHeight / 2};

        Vector2 mouseWorld = GetScreenToWorld2D(GetVirtualMousePosition(player.State), camera);
        Vector2 aimDir = Vector2Normalize(Vector2Subtract(mouseWorld, playerCenter));

        // Memfilter objek untuk hanya memeriksa yang ada di layer Interaction
        std::vector<MapObject> propObjects;
        for (auto &obj : tilesonMap->Objects)
        {
            if (obj.layerName == OBJECT_LAYER_NAME)
                propObjects.push_back(obj);
        }

        player.LastHit = player.Ray.Cast(playerCenter, aimDir, player.INTERACT_RANGE, propObjects);
    }

    /**
     * Memeriksa apakah pemain berdiri di dalam pemicu (trigger) pintu dan menekan tombol interaksi.
     */
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

            // Mengambil map tujuan dan ID pintu dari properti Tiled
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

    /**
     * Menangani interaksi dengan properti (peti, tuas, dll.) yang terkena raycast.
     */
    void CheckProps(Player &player)
    {
        if (!player.LastHit.hit)
            return;
        if (!InputInstance.IsInteract())
            return;

        const std::string &type = player.LastHit.object->type;
        TraceLog(LOG_INFO, "Berinteraksi dengan: '%s' (tipe: %s)", player.LastHit.object->name.c_str(), type.c_str());

        // Percabangan logika berdasarkan tipe objek
        if (type == CHEST_TYPE_OBJECT_NAME)
        {
            TraceLog(LOG_INFO, "Membuka peti: '%s'", player.LastHit.object->name.c_str());
        }
    }
}
