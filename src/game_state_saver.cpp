/**
 * @file game_state_saver.cpp
 * @brief Implementasi Modul Preservasi State Game
 *
 * Handle save dan restore seluruh state game world.
 */

#include "../include/game_state_saver.h"
#include "../include/map.h"

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
    
    for (int i = 0; i < 4; i++) {
        savedPlayerState.hotbar[i] = PlayerInstance.GetHotbarItem(i);
    }
    
    /*==============================================================================
     * Save Enemy States
     *==============================================================================*/
    savedEnemyStates.clear();
    for (const Enemy& enemy : activeEnemies) {
        SavedEnemyState savedEnemy;
        savedEnemy.position = enemy.position;
        savedEnemy.currentHP = enemy.currentHP;
        savedEnemy.isAlive = enemy.isAlive;
        savedEnemy.type = enemy.type;
        savedEnemyStates.push_back(savedEnemy);
    }
    
    /*==============================================================================
     * Save Item States
     *==============================================================================*/
    savedItemStates.clear();
    for (const Item& item : activeItems) {
        SavedItemState savedItem;
        savedItem.position = item.position;
        savedItem.isPickedUp = item.isPickedUp;
        savedItem.category = item.category;
        savedItem.rarity = item.rarity;
        savedItem.statMultiplier = item.statMultiplier;
        savedItemStates.push_back(savedItem);
    }
    
    /*==============================================================================
     * Save Map State (map path, camera, chest opened status)
     *==============================================================================*/
    const char* mapPath = GetCurrentMapPath();
    savedMapState.mapPath = (mapPath == nullptr || mapPath[0] == '\0') ? "world_json/tutorial.json" : std::string(mapPath);
    savedMapState.cameraTarget = camera.target;
    savedMapState.cameraZoom = camera.zoom;
    savedMapState.chestOpened.clear();
    if (tilesonMap != nullptr) {
        for (const MapObject& obj : tilesonMap->Objects) {
            if (obj.type == "chest") {
                savedMapState.chestOpened.push_back(0);
            }
        }
    }
    
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
    if (hasSavedState) {
        PlayerInstance.SetHealth(savedPlayerState.health);
        PlayerInstance.SetMana(savedPlayerState.mana);
        PlayerInstance.SetPosition(savedPlayerState.position);
        
        for (int i = 0; i < 4; i++) {
            PlayerInstance.SetHotbarItem(i, savedPlayerState.hotbar[i]);
        }
    }
    
    /*==============================================================================
     * Restore Enemy States
     *==============================================================================*/
    if (hasSavedState && !savedEnemyStates.empty()) {
        int enemyIndex = 0;
        for (Enemy& enemy : activeEnemies) {
            if (enemyIndex < (int)savedEnemyStates.size()) {
                enemy.currentHP = savedEnemyStates[enemyIndex].currentHP;
                enemy.isAlive = savedEnemyStates[enemyIndex].isAlive;
                enemy.position = savedEnemyStates[enemyIndex].position;
                enemyIndex++;
            }
        }
    }
    
    /*==============================================================================
     * Restore Item States
     *==============================================================================*/
    if (hasSavedState && !savedItemStates.empty()) {
        int itemIndex = 0;
        for (Item& item : activeItems) {
            if (itemIndex < (int)savedItemStates.size()) {
                item.isPickedUp = savedItemStates[itemIndex].isPickedUp;
                item.position = savedItemStates[itemIndex].position;
                item.category = savedItemStates[itemIndex].category;
                item.rarity = savedItemStates[itemIndex].rarity;
                item.statMultiplier = savedItemStates[itemIndex].statMultiplier;
                itemIndex++;
            }
        }
    }
    
    /*==============================================================================
     * Restore Map State (camera, chest opened status)
     *==============================================================================*/
    if (hasSavedState) {
        // Restore camera position
        camera.target = savedMapState.cameraTarget;
        camera.zoom = savedMapState.cameraZoom;
        // Chest state restoration skipped for now - requires more complex tileson handling
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
    for (int i = 0; i < 4; i++) {
        savedPlayerState.hotbar[i].type = ITEM_NONE;
        savedPlayerState.hotbar[i].name = "";
        savedPlayerState.hotbar[i].amount = 0;
        savedPlayerState.hotbar[i].damage = 0;
        savedPlayerState.hotbar[i].healValue = 0;
        savedPlayerState.hotbar[i].iconX = 0;
        savedPlayerState.hotbar[i].iconY = 0;
    }
    savedEnemyStates.clear();
    savedItemStates.clear();
    savedMapState.chestOpened.clear();
}