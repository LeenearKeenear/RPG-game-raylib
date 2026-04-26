/**
 * @file loading_screen.cpp
 * @brief Implementasi Modul Loading Screen
 *
 * Handle tampilan loading screen dan sequence loading asset game.
 * Asset dimuat sekali saja saat pertama kali Start Game, kemudian reuse.
 */

#include "../include/loading_screen.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../include/enemy.h"
#include "../include/item.h"
#include "../include/mainMenu.h"
#include "../include/game_state_saver.h"
#include "../lib/raylib/include/raylib.h"

/*==============================================================================
 * Konstanta Loading
 *==============================================================================*/

/** @brief Total stage loading */
#define TOTAL_LOADING_STAGES 6

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
    state->loadingProgress = 0.0f;
    state->loadingComplete = false;
    
    if (state->assetsLoaded) {
        state->loadingText = "Loading saved state...";
    } else {
        state->loadingText = "Starting asset loading...";
    }
}

/**
 * @brief UpdateLoadingScreen()
 * Update logic loading screen dan sequence loading asset.
 * @param state Pointer ke GameState
 * @details Load asset per stage, skip jika assetsLoaded sudah true
 */
void UpdateLoadingScreen(GameState *state)
{
    UpdateGame(state);
    
    if (state->loadingComplete) {
        return;
    }
    
    /*==============================================================================
     * Jika asset sudah dimuat, skip loading stages
     *==============================================================================*/
    if (state->assetsLoaded) {
        state->loadingStage = TOTAL_LOADING_STAGES;
        state->loadingProgress = 100.0f;
        state->loadingComplete = true;
        state->currentScreen = PLAY;
        
        RestoreGameState(state);
        InitAll();
        InitMainMenu(state);
        return;
    }
    
    /*==============================================================================
     * Loading sequence untuk pertama kali
     *==============================================================================*/
    switch (state->loadingStage) {
        case 0:
            state->loadingText = "Loading tilemap textures...";
            LoadTileTexture(TEXTURE_TILEMAP, "texture/tiles.png");
            state->loadingStage++;
            state->loadingProgress = (float)state->loadingStage / TOTAL_LOADING_STAGES * 100.0f;
            break;
            
        case 1:
            state->loadingText = "Loading character sprites...";
            LoadTileTexture(TEXTURE_KNIGHT, "texture/knight.png");
            state->loadingStage++;
            state->loadingProgress = (float)state->loadingStage / TOTAL_LOADING_STAGES * 100.0f;
            break;
            
        case 2:
            state->loadingText = "Loading item icons...";
            LoadTileTexture(TEXTURE_ITEMS, "texture/test.png");
            state->loadingStage++;
            state->loadingProgress = (float)state->loadingStage / TOTAL_LOADING_STAGES * 100.0f;
            break;
            
        case 3:
            state->loadingText = "Loading enemy textures...";
            LoadTileTexture(TEXTURE_ENEMIES, "texture/Enemies.png");
            state->loadingStage++;
            state->loadingProgress = (float)state->loadingStage / TOTAL_LOADING_STAGES * 100.0f;
            break;
            
        case 4:
            state->loadingText = "Loading map data...";
            InitMap();
            state->loadingStage++;
            state->loadingProgress = (float)state->loadingStage / TOTAL_LOADING_STAGES * 100.0f;
            break;
            
        case 5:
            state->loadingText = "Finalizing game assets...";
            state->loadingStage++;
            state->loadingProgress = (float)state->loadingStage / TOTAL_LOADING_STAGES * 100.0f;
            break;
            
        default:
            state->loadingComplete = true;
            state->assetsLoaded = true;
            state->loadingText = "Loading complete!";
            state->currentScreen = PLAY;
            
            RestoreGameState(state);
            InitAll();
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
    DrawText(state->loadingText, GameScreenWidth/2 - textWidth/2, GameScreenHeight/2 - 20, 20, WHITE);
    
    DrawRectangle(GameScreenWidth/2 - 150, GameScreenHeight/2 + 20, 300, 20, DARKGRAY);
    DrawRectangle(GameScreenWidth/2 - 150, GameScreenHeight/2 + 20, (int)(state->loadingProgress * 3), 20, GREEN);
    
    char progressText[10];
    sprintf(progressText, "%d%%", (int)state->loadingProgress);
    int progressTextWidth = MeasureText(progressText, 20);
    DrawText(progressText, GameScreenWidth/2 - progressTextWidth/2, GameScreenHeight/2 + 50, 20, WHITE);
    
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