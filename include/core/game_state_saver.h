#pragma once

/**
 * @file game_state_saver.h
 * @brief Modul Preservasi State Game
 *
 * Handle save dan restore seluruh state game world:
 * - Player (posisi, health, inventory, mana)
 * - Enemy (posisi, HP, alive/dead status)
 * - Item (posisi, collected/remaining status)
 * - Map state (opened chests, triggered events)
 */

#include "screen.h"
#include "player.h"
#include "enemy.h"
#include "item.h"
#include "map.h"
#include "inventory.h"
#include "mapstack.h"
#include <vector>
#include <string>

/*==============================================================================
 * Saved State Structures
 *==============================================================================*/

/**
 * @brief Struktur data untuk menyimpan state player
 */
struct SavedPlayerState {
    Vector2 position;           /**< Posisi player di world */
    float health;               /**< HP player saat ini */
    float mana;                 /**< Mana player saat ini */
    float maxHealth;            /**< Max HP player */
    float maxMana;              /**< Max Mana player */
    InventoryItem hotbar[4];    /**< Hotbar inventory player */
    InventoryItem bag[12];      /**< Full bag inventory (12 slots) */

    /**
     * @brief Snapshot animasi dan input state
     */
    struct {
        int state;              /**< Animation state (State enum) */
        int direction;          /**< Animation direction (Direction enum) */
        bool isDead;            /**< Apakah player mati */
        int activeSlot;         /**< Active hotbar slot (ItemSlot enum) */
    } animState;
};

/**
 * @brief Struktur data untuk menyimpan state satu enemy
 */
struct SavedEnemyState {
    Vector2 position;           /**< Posisi enemy di world */
    std::string enemyName;       /**< Nama tipe enemy ("Slime"/"Skeleton"/"Wolf") */
    int currentHP;              /**< HP enemy saat ini */
    bool isAlive;               /**< Status hidup/mati enemy */
    float maxHealth;            /**< Max HP enemy */
    int aiState;                /**< AI state (EnemyAIState enum: IDLE/PATROL/CHASE/ATTACK/RETURN) */
    float patrolTargetX;        /**< Target patroli X */
    float patrolTargetY;        /**< Target patroli Y */
    float patrolTimer;          /**< Timer tunggu patroli */
    int mapObjectID;            /**< MapObjectID untuk matching saat restore */
};

/**
 * @brief Struktur data untuk menyimpan state satu item
 * 
 * Hanya menyimpan state yang unik per instance.
 * Data statis item (nama, kategori, rarity, dll) diambil dari
 * ItemDefinitionManager via definitionId.
 */
struct SavedItemState {
    Vector2 position;   /**< Posisi item di world */
    bool isPickedUp;    /**< Status collected/remaining */
    int definitionId;   /**< ID referensi ke ItemDefinition */
};

/**
 * @brief Struktur data untuk menyimpan state map (chest, dll)
 */
struct SavedMapState {
    std::string mapPath;                              /**< Path map saat ini */
    Vector2 cameraTarget;                          /**< Posisi camera target */
    float cameraZoom;                            /**< Zoom camera */
    std::vector<unsigned char> chestOpened;                   /**< Status opened/closed tiap chest */
    std::vector<std::string> deadEntities;                    /**< Nama entitas yang sudah mati di map ini */
    std::vector<std::string> chestsOpened;                    /**< Posisi chest yang sudah dibuka (encoded string) */
    std::vector<MapSystem::MapHistoryEntry> mapHistory;       /**< Stack riwayat perpindahan map */
};

/*==============================================================================
 * Global Saved State Variables
 *==============================================================================*/

/**
 * @brief State player yang tersimpan
 */
extern SavedPlayerState savedPlayerState;

/**
 * @brief Daftar state enemy yang tersimpan
 */
extern std::vector<SavedEnemyState> savedEnemyStates;

/**
 * @brief Daftar state item yang tersimpan
 */
extern std::vector<SavedItemState> savedItemStates;

/**
 * @brief State map yang tersimpan
 */
extern SavedMapState savedMapState;

/**
 * @brief Flag menandakan apakah ada state tersimpan
 */
extern bool hasSavedState;

/*==============================================================================
 * State Save/Restore Functions
 *==============================================================================*/

/**
 * @brief Simpan seluruh state game world
 * @details Dipanggil saat player klik "Return to Menu"
 * @param state Pointer ke GameState
 * @note Simpan: player position/health/inventory, enemies, items, map state
 */
void SaveGameState(GameState *state);

/**
 * @brief Kembalikan seluruh state game world
 * @details Dipanggil saat masuk PLAY state dari LOADING
 * @param state Pointer ke GameState
 * @note Restore: player, enemies, items, map state dari data tersimpan
 */
void RestoreGameState(GameState *state);

/**
 * @brief Cek apakah ada state tersimpan
 * @return true jika ada state yang bisa direstore
 * @note Dipakai untuk判断 apakah ini new game atau resume
 */
bool HasSavedState(void);

/**
 * @brief Bersihkan state tersimpan
 * @details Untuk new game dari awal (fresh start)
 */
void ClearSavedState(void);

/*==============================================================================
 * File I/O Functions
 *==============================================================================*/

/**
 * @brief Write saved state to JSON file
 * @details Serializes all global saved state structs to a JSON file.
 *          Uses atomic write: writes to .tmp then renames.
 * @param path Path to the save file
 */
void WriteSaveFile(const std::string& path);

/**
 * @brief Read saved state from JSON file
 * @details Reads and deserializes a JSON save file into the global saved state structs.
 *          Validates version == 1.
 * @param path Path to the save file
 * @return true if successful, false if file not found, parse error, or version mismatch
 */
bool ReadSaveFile(const std::string& path);

/**
 * @brief Check if a save file exists and has content
 * @param path Path to the save file
 * @return true if file exists and has non-zero size
 */
bool HasSaveFile(const std::string& path);

/**
 * @brief Delete a save file if it exists
 * @param path Path to the save file to delete
 */
void DeleteSaveFile(const std::string& path);