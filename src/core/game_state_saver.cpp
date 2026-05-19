/**
 * @file game_state_saver.cpp
 * @brief Implementasi Modul Preservasi State Game
 *
 * Handle save dan restore seluruh state game world.
 */

#include "game_state_saver.h"
#include "map.h"
#include "propsbehavior.h"
#include "entities.h"
#include "../lib/json/include/nlohmann/json.hpp"
#include <fstream>
#include <filesystem>
#include <ctime>

using json = nlohmann::json;

/*==============================================================================
 * Global Saved State Variables
 *==============================================================================*/

/**
 * @brief State player yang tersimpan
 */
SavedPlayerState savedPlayerState;

/**
 * @brief Daftar state enemy yang tersimpan
 */
std::vector<SavedEnemyState> savedEnemyStates;

/**
 * @brief Daftar state item yang tersimpan
 */
std::vector<SavedItemState> savedItemStates;

/**
 * @brief State map yang tersimpan
 */
SavedMapState savedMapState;

/**
 * @brief Flag menandakan apakah ada state tersimpan
 */
bool hasSavedState = false;

/*==============================================================================
 * File I/O Functions
 *==============================================================================*/

/**
 * WriteSaveFile - Serialize all global saved state to a JSON file using atomic write.
 */
void WriteSaveFile(const std::string& path)
{
    json root;

    // Version and timestamp metadata
    root["version"] = 1;
    std::time_t t = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", std::gmtime(&t));
    root["timestamp"] = std::string(buf);

    // Player section
    json playerJson;
    playerJson["position"] = {savedPlayerState.position.x, savedPlayerState.position.y};
    playerJson["health"] = savedPlayerState.health;
    playerJson["maxHealth"] = savedPlayerState.maxHealth;
    playerJson["mana"] = savedPlayerState.mana;
    playerJson["maxMana"] = savedPlayerState.maxMana;

    json hotbarJson = json::array();
    for (int i = 0; i < 4; i++)
    {
        json item;
        item["definitionId"] = savedPlayerState.hotbar[i].definitionId;
        item["amount"] = savedPlayerState.hotbar[i].amount;
        hotbarJson.push_back(item);
    }
    playerJson["hotbar"] = hotbarJson;

    json bagJson = json::array();
    for (int i = 0; i < 12; i++)
    {
        json item;
        item["definitionId"] = savedPlayerState.bag[i].definitionId;
        item["amount"] = savedPlayerState.bag[i].amount;
        bagJson.push_back(item);
    }
    playerJson["bag"] = bagJson;

    json animJson;
    animJson["state"] = savedPlayerState.animState.state;
    animJson["direction"] = savedPlayerState.animState.direction;
    animJson["isDead"] = savedPlayerState.animState.isDead;
    animJson["activeSlot"] = savedPlayerState.animState.activeSlot;
    playerJson["animState"] = animJson;

    playerJson["showFPS"] = gState->showFPS;

    root["player"] = playerJson;

    // Enemies section
    json enemiesJson = json::array();
    for (const auto& enemy : savedEnemyStates)
    {
        json e;
        e["position"] = {enemy.position.x, enemy.position.y};
        e["enemyName"] = enemy.enemyName;
        e["currentHP"] = enemy.currentHP;
        e["isAlive"] = enemy.isAlive;
        enemiesJson.push_back(e);
    }
    root["enemies"] = enemiesJson;

    // Items section
    json itemsJson = json::array();
    for (const auto& item : savedItemStates)
    {
        json it;
        it["position"] = {item.position.x, item.position.y};
        it["isPickedUp"] = item.isPickedUp;
        it["definitionId"] = item.definitionId;
        itemsJson.push_back(it);
    }
    root["items"] = itemsJson;

    // Map section
    json mapJson;
    mapJson["mapPath"] = savedMapState.mapPath;
    mapJson["cameraTarget"] = {savedMapState.cameraTarget.x, savedMapState.cameraTarget.y};
    mapJson["cameraZoom"] = savedMapState.cameraZoom;
    mapJson["chestOpened"] = savedMapState.chestOpened;

    json deadEntitiesJson = json::array();
    for (const auto& name : savedMapState.deadEntities)
        deadEntitiesJson.push_back(name);
    mapJson["deadEntities"] = deadEntitiesJson;

    json chestsOpenedJson = json::array();
    for (const auto& pos : savedMapState.chestsOpened)
        chestsOpenedJson.push_back(pos);
    mapJson["chestsOpened"] = chestsOpenedJson;

    json mapHistoryJson = json::array();
    for (const auto& entry : savedMapState.mapHistory)
    {
        json h;
        h["mapPath"] = entry.mapPath;
        h["doorName"] = entry.doorName;
        mapHistoryJson.push_back(h);
    }
    mapJson["mapHistory"] = mapHistoryJson;

    root["map"] = mapJson;

    // Atomic write: write to .tmp then rename
    std::string tmpPath = path + ".tmp";
    std::ofstream file(tmpPath);
    file << root.dump(4);
    file.close();

    std::filesystem::rename(tmpPath, path);
}

/**
 * ReadSaveFile - Read JSON save file and deserialize into global saved state.
 */
bool ReadSaveFile(const std::string& path)
{
    if (!std::filesystem::exists(path))
        return false;

    try
    {
        std::ifstream file(path);
        json root = json::parse(file);

        // Validate version
        int version = root.at("version").get<int>();
        if (version != 1)
            return false;

        // Read player section
        const auto& player = root.at("player");
        savedPlayerState.position.x = player.at("position")[0].get<float>();
        savedPlayerState.position.y = player.at("position")[1].get<float>();
        savedPlayerState.health = player.value("health", 100.0f);
        savedPlayerState.maxHealth = player.value("maxHealth", 100.0f);
        savedPlayerState.mana = player.value("mana", 100.0f);
        savedPlayerState.maxMana = player.value("maxMana", 100.0f);

        if (player.contains("hotbar"))
        {
            const auto& hotbar = player.at("hotbar");
            for (int i = 0; i < 4 && i < (int)hotbar.size(); i++)
            {
                savedPlayerState.hotbar[i].definitionId = hotbar[i].value("definitionId", -1);
                savedPlayerState.hotbar[i].amount = hotbar[i].value("amount", 0);
            }
        }

        if (player.contains("bag"))
        {
            const auto& bag = player.at("bag");
            for (int i = 0; i < 12 && i < (int)bag.size(); i++)
            {
                savedPlayerState.bag[i].definitionId = bag[i].value("definitionId", -1);
                savedPlayerState.bag[i].amount = bag[i].value("amount", 0);
            }
        }

        if (player.contains("animState"))
        {
            const auto& anim = player.at("animState");
            savedPlayerState.animState.state = anim.value("state", 0);
            savedPlayerState.animState.direction = anim.value("direction", 0);
            savedPlayerState.animState.isDead = anim.value("isDead", false);
            savedPlayerState.animState.activeSlot = anim.value("activeSlot", 0);
        }

        if (player.contains("showFPS"))
            gState->showFPS = player.at("showFPS").get<bool>();

        // Read enemies section
        savedEnemyStates.clear();
        if (root.contains("enemies"))
        {
            for (const auto& e : root.at("enemies"))
            {
                SavedEnemyState enemy;
                enemy.position.x = e.at("position")[0].get<float>();
                enemy.position.y = e.at("position")[1].get<float>();
                enemy.enemyName = e.value("enemyName", "");
                enemy.currentHP = e.value("currentHP", 0);
                enemy.isAlive = e.value("isAlive", true);
                savedEnemyStates.push_back(enemy);
            }
        }

        // Read items section
        savedItemStates.clear();
        if (root.contains("items"))
        {
            for (const auto& it : root.at("items"))
            {
                SavedItemState item;
                item.position.x = it.at("position")[0].get<float>();
                item.position.y = it.at("position")[1].get<float>();
                item.isPickedUp = it.value("isPickedUp", false);
                item.definitionId = it.value("definitionId", -1);
                savedItemStates.push_back(item);
            }
        }

        // Read map section
        const auto& map = root.at("map");
        savedMapState.mapPath = map.value("mapPath", "");
        savedMapState.cameraTarget.x = map.at("cameraTarget")[0].get<float>();
        savedMapState.cameraTarget.y = map.at("cameraTarget")[1].get<float>();
        savedMapState.cameraZoom = map.value("cameraZoom", 1.0f);

        savedMapState.chestOpened.clear();
        if (map.contains("chestOpened"))
        {
            for (const auto& c : map.at("chestOpened"))
                savedMapState.chestOpened.push_back(static_cast<unsigned char>(c.get<int>()));
        }

        savedMapState.deadEntities.clear();
        if (map.contains("deadEntities"))
        {
            for (const auto& d : map.at("deadEntities"))
                savedMapState.deadEntities.push_back(d.get<std::string>());
        }

        savedMapState.chestsOpened.clear();
        if (map.contains("chestsOpened"))
        {
            for (const auto& c : map.at("chestsOpened"))
                savedMapState.chestsOpened.push_back(c.get<std::string>());
        }

        savedMapState.mapHistory.clear();
        if (map.contains("mapHistory"))
        {
            for (const auto& h : map.at("mapHistory"))
            {
                MapSystem::MapHistoryEntry entry;
                entry.mapPath = h.value("mapPath", "");
                entry.doorName = h.value("doorName", "");
                savedMapState.mapHistory.push_back(entry);
            }
        }

        hasSavedState = true;
        return true;
    }
    catch (const json::parse_error&)
    {
        return false;
    }
    catch (const json::out_of_range&)
    {
        return false;
    }
}

/**
 * HasSaveFile - Check if save file exists and has content.
 */
bool HasSaveFile(const std::string& path)
{
    return std::filesystem::exists(path) && std::filesystem::file_size(path) > 0;
}

/**
 * DeleteSaveFile - Remove save file if it exists.
 */
void DeleteSaveFile(const std::string& path)
{
    if (std::filesystem::exists(path))
        std::filesystem::remove(path);
}

/*==============================================================================
 * State Save/Restore Functions
 *==============================================================================*/

/**
 * @brief SaveGameState()
 * Simpan seluruh state game world saat kembali ke menu.
 * @param state Pointer ke GameState
 */
void SaveGameState(GameState *state)
{
    /*==============================================================================
     * Save Player State
     *==============================================================================*/
    savedPlayerState.position = PlayerInstance.GetPosition();
    savedPlayerState.health = PlayerInstance.GetHealth();
    savedPlayerState.mana = PlayerInstance.GetMana();

    for (int i = 0; i < 4; i++)
    {
        savedPlayerState.hotbar[i] = PlayerInstance.GetHotbarItem(i);
    }

    // Save new fields: max stats, inventory bag, animation state, active slot
    savedPlayerState.maxHealth = PlayerInstance.MaxHealth;
    savedPlayerState.maxMana = PlayerInstance.MaxMana;

    for (int i = 0; i < 12; i++)
    {
        savedPlayerState.bag[i] = PlayerInstance.GetBagItem(i);
    }

    savedPlayerState.animState.state = PlayerInstance.Anim.state;
    savedPlayerState.animState.direction = PlayerInstance.Anim.direction;
    savedPlayerState.animState.isDead = PlayerInstance.Anim.isDead;
    savedPlayerState.animState.activeSlot = InputInstance.GetActiveSlot();

    /*==============================================================================
     * Save Enemy States
     *==============================================================================*/
    savedItemStates.clear();
    for (const ItemSpawn &item : itemData.activeItems)
    {
        SavedItemState savedItem;
        savedItem.position = item.position;
        savedItem.isPickedUp = item.isPickedUp;
        savedItem.definitionId = item.definitionId;
        savedItemStates.push_back(savedItem);
    }

    /*==============================================================================
     * Save Map State (map path, camera, chest opened status)
     *==============================================================================*/
    const char *mapPath = GetCurrentMapPath();
    savedMapState.mapPath = (mapPath == nullptr || mapPath[0] == '\0') ? "assets/maps/tutorial.json" : std::string(mapPath);
    savedMapState.cameraTarget = camera.target;
    savedMapState.cameraZoom = camera.zoom;
    savedMapState.chestOpened.clear();
    if (tilesonMap != nullptr)
    {
        for (const MapObject &obj : tilesonMap->Objects)
        {
            if (obj.type == "chest")
            {
                savedMapState.chestOpened.push_back(0);
            }
        }
    }

    /*==============================================================================
     * Save Dead Entities (names of entities killed in this map)
     *==============================================================================*/
    savedMapState.deadEntities.clear();
    {
        const auto &deadSet = Entities::GetDeadEntities();
        savedMapState.deadEntities.assign(deadSet.begin(), deadSet.end());
    }

    /*==============================================================================
     * Save Consumed Chest Positions (chests already opened)
     *==============================================================================*/
    savedMapState.chestsOpened.clear();
    {
        const auto &consumed = chestManager.GetConsumedPositions();
        savedMapState.chestsOpened.assign(consumed.begin(), consumed.end());
    }

    /*==============================================================================
     * Save Map History (stack of visited maps)
     *==============================================================================*/
    savedMapState.mapHistory = mapHistoryStack.GetAllEntries();

    /*==============================================================================
     * Write save file to disk
     *==============================================================================*/
    WriteSaveFile("saves/manual/slot0.json");

    /*==============================================================================
     * Mark as having saved state
     *==============================================================================*/
    hasSavedState = true;
}

/**
 * @brief RestoreGameState()
 * Kembalikan seluruh state game world saat masuk gameplay.
 * @param state Pointer ke GameState
 */
void RestoreGameState(GameState *state)
{
    /*==============================================================================
     * Restore Player State
     *==============================================================================*/
    if (hasSavedState)
    {
        // Restore max stats first so SetHealth/SetMana can clamp correctly
        if (savedPlayerState.maxHealth > 0)
        {
            PlayerInstance.MaxHealth = savedPlayerState.maxHealth;
            PlayerInstance.MaxMana = savedPlayerState.maxMana;
        }
        else
        {
            PlayerInstance.MaxHealth = 100.0f;
            PlayerInstance.MaxMana = 100.0f;
        }

        PlayerInstance.SetHealth(savedPlayerState.health);
        PlayerInstance.SetMana(savedPlayerState.mana);
        PlayerInstance.SetPosition(savedPlayerState.position);

        for (int i = 0; i < 4; i++)
        {
            PlayerInstance.SetHotbarItem(i, savedPlayerState.hotbar[i]);
        }

        // Restore bag inventory
        for (int i = 0; i < 12; i++)
        {
            PlayerInstance.GetBagItem(i) = savedPlayerState.bag[i];
        }

        // Restore animation state
        PlayerInstance.Anim.state = static_cast<State>(savedPlayerState.animState.state);
        PlayerInstance.Anim.direction = static_cast<Direction>(savedPlayerState.animState.direction);
        PlayerInstance.Anim.isDead = savedPlayerState.animState.isDead;

        // Restore active slot
        InputInstance.SetActiveSlot(static_cast<ItemSlot>(savedPlayerState.animState.activeSlot));
    }

    /*==============================================================================
     * Restore Enemy States
     *==============================================================================
     * @note Enemy restore dinonaktifkan karena sistem enemy telah diubah
     *   menggunakan Entities namespace di codebase baru. Untuk sekarang,
     *   enemy akan di-spawn ulang setiap kali masuk game.
     *   TODO: Update untuk menggunakan Entities system di masa depan.
     */
    // if (hasSavedState && !savedEnemyStates.empty()) {
    //     int enemyIndex = 0;
    //     for (Enemy& enemy : activeEnemies) {
    //         if (enemyIndex < (int)savedEnemyStates.size()) {
    //             enemy.currentHP = savedEnemyStates[enemyIndex].currentHP;
    //             enemy.isAlive = savedEnemyStates[enemyIndex].isAlive;
    //             enemy.position = savedEnemyStates[enemyIndex].position;
    //             enemyIndex++;
    //         }
    //     }
    // }

    /*==============================================================================
     * Restore Item States
     *==============================================================================*/
    if (hasSavedState && !savedItemStates.empty())
    {
        int itemIndex = 0;
        for (ItemSpawn &item : itemData.activeItems)
        {
            if (itemIndex < (int)savedItemStates.size())
            {
                item.isPickedUp = savedItemStates[itemIndex].isPickedUp;
                item.position = savedItemStates[itemIndex].position;
                item.definitionId = savedItemStates[itemIndex].definitionId;
                itemIndex++;
            }
        }
    }
    /*==============================================================================
     * Restore Map State (camera, chest opened status)
     *==============================================================================*/
    if (hasSavedState)
    {
        // Restore camera position
        camera.target = savedMapState.cameraTarget;
        camera.zoom = savedMapState.cameraZoom;

        // Restore dead entities
        if (!savedMapState.deadEntities.empty())
        {
            Entities::SetDeadEntities(std::set<std::string>(
                savedMapState.deadEntities.begin(),
                savedMapState.deadEntities.end()));
        }

        // Restore consumed chest positions
        if (!savedMapState.chestsOpened.empty())
        {
            chestManager.SetConsumedPositions(std::unordered_set<std::string>(
                savedMapState.chestsOpened.begin(),
                savedMapState.chestsOpened.end()));
        }

        // Restore map history
        if (!savedMapState.mapHistory.empty())
        {
            mapHistoryStack.FromVector(savedMapState.mapHistory);
        }
    }
}

/**
 * @brief HasSavedState()
 * Cek apakah ada state tersimpan.
 * @return true jika ada state yang bisa direstore
 */
bool HasSavedState(void)
{
    return hasSavedState;
}

/**
 * @brief ClearSavedState()
 * Bersihkan state tersimpan untuk fresh start.
 */
void ClearSavedState(void)
{
    hasSavedState = false;
    savedPlayerState.position = {0};
    savedPlayerState.health = 0;
    savedPlayerState.mana = 0;
    savedPlayerState.maxHealth = 0;
    savedPlayerState.maxMana = 0;
    for (int i = 0; i < 4; i++)
        savedPlayerState.hotbar[i] = {-1, 0};
    for (int i = 0; i < 12; i++)
        savedPlayerState.bag[i] = {-1, 0};
    savedPlayerState.animState = {0};
    savedEnemyStates.clear();
    savedItemStates.clear();
    savedMapState.chestOpened.clear();
    savedMapState.deadEntities.clear();
    savedMapState.chestsOpened.clear();
    savedMapState.mapHistory.clear();
}
