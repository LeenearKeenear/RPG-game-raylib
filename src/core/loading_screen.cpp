/**
 * @file loading_screen.cpp
 * @brief Implementasi Modul Loading Screen
 *
 * Handle tampilan loading screen dan sequence loading asset game.
 * Asset dimuat sekali saja saat pertama kali Start Game, kemudian reuse.
 * Juga menangani transisi map dengan loading screen yang sama.
 */

#include "../../include/core/loading_screen.h"
#include "../../include/map/map.h"
#include "../../include/entities/player.h"
#include "../../include/entities/enemy.h"
#include "../../include/items/item.h"
#include "../../include/ui/mainMenu.h"
#include "../../include/core/game_state_saver.h"
#include "../../include/core/screen.h"
#include "../../include/entities/entities.h"
#include "../../include/map/propsbehavior.h"
#include "../../include/entities/enemy_ai.h"
#include "../../include/rendering/fonts.h"
#include "../../lib/raylib/include/raylib.h"

/*==============================================================================
 * Konstanta Loading
 *==============================================================================*/

/** @brief Total stage loading untuk initial startup */
#define TOTAL_LOADING_STAGES 3

/** @brief Total stage loading untuk map switch */
#define MAP_SWITCH_STAGES 4

/*==============================================================================
 * Public Functions
 *==============================================================================*/

/**
 * @brief InitLoadingScreen()
 * Inisialisasi state loading screen.
 * @param state Pointer ke GameState
 * @details Reset loadingStage dan loadingProgress saat masuk LOADING state
 */
void InitLoadingScreen(GameState *state)
{
    state->enteredLoading = true;
    state->loadingStage = 0;
    state->loadingProgress = 0.0F;
    state->loadingComplete = false;

    if (state->isSwitchingMap)
    {
        state->loadingText = "Switching map...";
    }
    else if (state->assetsLoaded)
    {
        state->loadingText = "Loading saved state...";
    }
    else
    {
        state->loadingText = "Starting asset loading...";
    }
}

/**
 * @brief UpdateLoadingScreen()
 * Update logic loading screen dan sequence loading asset.
 * @param state Pointer ke GameState
 * @details Load asset per stage, skip jika assetsLoaded sudah true.
 *          Juga menangani map switch dengan stages terpisah.
 */
void UpdateLoadingScreen(GameState *state)
{
    UpdateGame(state);

    if (state->loadingComplete)
    {
        return;
    }

    /*==============================================================================
     * Map Switch Loading - handle transisi map dengan loading screen
     *==============================================================================*/
    if (state->isSwitchingMap || state->isGoingBack)
    {
        bool isBack = state->isGoingBack;

        switch (state->loadingStage)
        {
        case 0:
            TraceLog(LOG_INFO, "LOADING: [stage 1/4] %s", isBack ? "Returning to previous map" : "Unloading current map");
            state->loadingText = isBack ? "Returning to previous map..." : "Unloading current map...";
            UnloadMap();
            spawnFlowFields.clear();
            state->loadingStage++;
            state->loadingProgress = (float)state->loadingStage / MAP_SWITCH_STAGES * 100.0F;
            break;

        case 1:
            TraceLog(LOG_INFO, "LOADING: [stage 2/4] Loading map: %s", state->pendingMapPath.c_str());
            state->loadingText = isBack ? "Reloading previous map..." : "Loading new map...";
            LoadMap(state->pendingMapPath.c_str());
            SetCurrentMapPath(state->pendingMapPath.c_str());
            BuildMapObjectIndex();
            SpawnObject();
            RebuildObstacleCache();
            globalFlowField.Invalidate(); // nanti diganti kalo nambah method ai nya
            state->loadingStage++;
            state->loadingProgress = (float)state->loadingStage / MAP_SWITCH_STAGES * 100.0F;
            break;

        case 2:
            TraceLog(LOG_INFO, "LOADING: [stage 3/4] Initializing player and entities");
            state->loadingText = "Initializing player and entities...";
            // Re-init player berdasarkan target door di map baru
            PlayerInstance.Init(gState, state->pendingDoorName.c_str());

            // Bersihkan entitas map sebelumnya dan add player
            Entities::Clear();
            Entities::Add(&PlayerInstance);

            // Load musuh dari save per-map, atau spawn baru jika tak ada
            if (!LoadEnemiesForMap(state->pendingMapPath))
            {
                SpawnEnemiesFromMap();
            }

            // Load items from filesystem persistence
            if (!LoadItemsForMapDir(state->pendingMapPath))
            {
                SpawnItemWave();
            }

            state->loadingStage++;
            state->loadingProgress = (float)state->loadingStage / MAP_SWITCH_STAGES * 100.0F;
            break;

        case 3:
            TraceLog(LOG_INFO, "LOADING: [stage 4/4] Finalizing map switch");
            state->loadingText = "Finalizing map switch...";
            // Set camera ke spawn player
            Vector2 spawnPos = PlayerInstance.GetPosition();
            camera.target = {spawnPos.x + (FRAME_SIZE / 2.0F), spawnPos.y + (FRAME_SIZE / 2.0F)};
            camera.offset = {(float)(GameScreenWidth / 2), (float)(GameScreenHeight / 2)};
            camera.rotation = 0;
            camera.zoom = 1.0F;
            Movement::UpdateCamera(PlayerInstance);

            // Update current map path
            SetCurrentMapPath(state->pendingMapPath.c_str());

            // Clear map switch state
            state->isSwitchingMap = false;
            state->isGoingBack = false;
            state->pendingMapPath.clear();
            state->pendingDoorName.clear();

            TraceLog(LOG_INFO, "LOADING: Map switch complete, player at (%.2f, %.2f)", PlayerInstance.GetPosition().x, PlayerInstance.GetPosition().y);
            WriteAutosave("quick.json");
            state->loadingComplete = true;
            state->loadingProgress = 100.0F;
            state->loadingText = "Map loaded!";
            state->currentScreen = PLAY;
            break;
        }
        return;
    }

    /*==============================================================================
     * Jika asset sudah dimuat, skip loading stages
     *==============================================================================*/
    if (state->assetsLoaded)
    {
        TraceLog(LOG_INFO, "LOADING: Fast path (assets already loaded)");

        state->loadingStage = TOTAL_LOADING_STAGES;
        state->loadingProgress = 100.0F;
        state->loadingComplete = true;
        state->currentScreen = PLAY;

        // Free previous map allocation before loading the correct one
        UnloadMap();

        if (HasSavedState())
        {
            if (!savedMapState.mapPath.empty())
            {
                LoadMap(savedMapState.mapPath.c_str());
                SetCurrentMapPath(savedMapState.mapPath.c_str());
                BuildMapObjectIndex();
            }
            else
            {
                InitMap();
            }
        }
        else
        {
            PlayerInstance.ResetForNewGame();
            InitMap();
        }

        // Restore dead entities BEFORE InitAll to prevent dead enemies respawning
        if (HasSavedState())
            RestoreDeadEntities();
        // Init first, then restore saved state - order matters!
        // InitAll() sets position to spawn, then RestoreGameState overwrites it
        InitAll();
        InitFonts();
        if (HasSavedState())
        {
            RestoreGameState(state);
            TraceLog(LOG_INFO, "LOADING: after RestoreGameState, player pos = (%.2f, %.2f)", PlayerInstance.GetPosition().x, PlayerInstance.GetPosition().y);
        }
        else
        {
            WriteAutosave("spawn.json");
        }
        // Safety net: deactivate any dead entities that survived spawn
        Entities::PruneDeadEntities();
        InitMainMenu(state);
        return;
    }

    /*==============================================================================
     * Loading sequence untuk pertama kali
     *==============================================================================*/
    switch (state->loadingStage)
    {
    case 0:
        state->loadingText = "Loading game textures...";
        InitTextures();
        state->loadingStage++;
        state->loadingProgress = (float)state->loadingStage / TOTAL_LOADING_STAGES * 100.0F;
        break;

    case 1:
        state->loadingText = "Loading map data...";
        // Load saved map if resuming, otherwise default
        if (HasSavedState() && !savedMapState.mapPath.empty())
        {
            LoadMap(savedMapState.mapPath.c_str());
            SetCurrentMapPath(savedMapState.mapPath.c_str());
            BuildMapObjectIndex();
        }
        else
        {
            InitMap();
        }
        state->loadingStage++;
        state->loadingProgress = (float)state->loadingStage / TOTAL_LOADING_STAGES * 100.0F;
        break;

    case 2:
        state->loadingText = "Finalizing game assets...";
        state->loadingStage++;
        state->loadingProgress = (float)state->loadingStage / TOTAL_LOADING_STAGES * 100.0F;
        break;

    default:
        state->loadingComplete = true;
        state->assetsLoaded = true;
        state->loadingText = "Loading complete!";
        state->currentScreen = PLAY;

        // Restore dead entities BEFORE InitAll to prevent dead enemies respawning
        if (HasSavedState())
            RestoreDeadEntities();
        // Initialize everything first, then restore saved state
        InitAll();
        if (HasSavedState())
        {
            RestoreGameState(state);
        }
        else
        {
            WriteAutosave("spawn.json");
        }
        // Safety net: deactivate any dead entities that survived spawn
        Entities::PruneDeadEntities();
        InitMainMenu(state);
        break;
    }
}

/**
 * @brief RenderLoadingScreen()
 * Render loading screen ke layar virtual.
 * @param state Pointer ke GameState
 */
void RenderLoadingScreen(GameState *state)
{
    BeginTextureMode(state->Dungeon);
    ClearBackground(DARKGRAY);

    int textWidth = MeasureText(state->loadingText, 20);
    DrawText(state->loadingText, (GameScreenWidth / 2) - (textWidth / 2), (GameScreenHeight / 2) - 20, 20, WHITE);

    DrawRectangle((GameScreenWidth / 2) - 150, (GameScreenHeight / 2) + 20, 300, 20, DARKGRAY);
    DrawRectangle((GameScreenWidth / 2) - 150, (GameScreenHeight / 2) + 20, (int)(state->loadingProgress * 3), 20, GREEN);

    std::array<char, 10> progressText;
    sprintf(progressText.data(), "%d%%", (int)state->loadingProgress);
    int progressTextWidth = MeasureText(progressText.data(), 20);
    DrawText(progressText.data(), (GameScreenWidth / 2) - (progressTextWidth / 2), (GameScreenHeight / 2) + 50, 20, WHITE);

    EndTextureMode();
}

/**
 * @brief IsLoadingComplete()
 * Memeriksa apakah loading sudah selesai.
 * @param state Pointer ke GameState
 * @return true jika loading selesai
 */
bool IsLoadingComplete(GameState *state)
{
    return state->loadingComplete;
}
