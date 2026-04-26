/**
 * @file loading_screen.cpp
 * @brief Implementasi dari Loading Screen Module
 * 
 * Handle loading screen display dan asset loading sequence.
 */

#include "../include/loading_screen.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../include/enemy.h"
#include "../include/item.h"
#include "../include/mainMenu.h"
#include "../lib/raylib/include/raylib.h"

/*==============================================================================
 * Static Variables
 *==============================================================================*/

/*==============================================================================
 * Public Functions
 *==============================================================================*/

/**
 * @brief InitLoadingScreen()
 * Inisialisasi loading screen state.
 * @param state GameState pointer
 */
void InitLoadingScreen(GameState *state)
{
    // Initialize loading state if not already done
    if (state->loadingProgress == 0.0f && !state->loadingComplete) {
        state->loadingText = "Starting asset loading...";
    }
}

/**
 * @brief UpdateLoadingScreen()
 * Update loading screen logic dan sequence asset loading.
 * @param state GameState pointer
 */
void UpdateLoadingScreen(GameState *state)
{
    // Update game state (for scaling, etc.)
    UpdateGame(state);
    
    // Asset loading sequence
    if (!state->loadingComplete) {
        // Actual asset loading implementation
        static int loadingStage = 0;
        const int totalStages = 6; // tiles, knight, items, enemies, logo, complete
        
        // Load assets based on current stage
        switch (loadingStage) {
            case 0: // Loading tileset textures
                state->loadingText = "Loading tileset textures...";
                // Load tile textures (these are loaded in InitMap via Tileson)
                // We'll simulate by just advancing since Tileson handles it
                loadingStage++;
                state->loadingProgress = (float)loadingStage / totalStages * 100.0f;
                break;
                
            case 1: // Loading character sprites
                state->loadingText = "Loading character sprites...";
                // Load knight texture (loaded in Player::Init)
                // We'll load it here to show progress
                if (state->loadingProgress < 33.0f) { // Only load once
                    LoadTileTexture(TEXTURE_KNIGHT, "texture/knight.png");
                }
                loadingStage++;
                state->loadingProgress = (float)loadingStage / totalStages * 100.0f;
                break;
                
            case 2: // Loading item icons
                state->loadingText = "Loading item icons...";
                // Load items texture (loaded in Player::Init and Item::InitItems)
                if (state->loadingProgress < 50.0f) { // Only load once
                    LoadTileTexture(TEXTURE_ITEMS, "texture/test.png");
                }
                loadingStage++;
                state->loadingProgress = (float)loadingStage / totalStages * 100.0f;
                break;
                
            case 3: // Loading enemy textures
                state->loadingText = "Loading enemy textures...";
                // Load enemies texture (loaded in Enemy::InitEnemy)
                if (state->loadingProgress < 66.0f) { // Only load once
                    LoadTileTexture(TEXTURE_ENEMIES, "texture/Enemies.png");
                }
                loadingStage++;
                state->loadingProgress = (float)loadingStage / totalStages * 100.0f;
                break;
                
            case 4: // Loading logo
                state->loadingText = "Loading game logo...";
                // Logo is loaded in main menu, but we can load it here too
                if (state->loadingProgress < 83.0f) { // Only load once
                    Image logoImg = LoadImage("texture/logo.png");
                    // We don't need to keep this texture as it's loaded in main menu
                    UnloadImage(logoImg);
                }
                loadingStage++;
                state->loadingProgress = (float)loadingStage / totalStages * 100.0f;
                break;
                
            case 5: // Finalizing
                state->loadingText = "Finalizing game assets...";
                loadingStage++;
                state->loadingProgress = (float)loadingStage / totalStages * 100.0f;
                break;
                
            default: // Complete
                state->loadingComplete = true;
                state->loadingText = "Loading complete!";
                // Transition to PLAY state after loading is complete
                state->currentScreen = PLAY;
                
                /*==============================================================================
                 * Post-Loading Initialization
                 *==============================================================================*/
                /**
                 * @brief Initialize game systems that should happen after asset loading
                 * @note These were moved from the main initialization sequence to occur after loading
                 */
                InitMap();
                InitAll();
                InitMainMenu(state);
                break;
        }
    }
}

/**
 * @brief RenderLoadingScreen()
 * Render loading screen ke layar virtual.
 * @param state GameState pointer
 */
void RenderLoadingScreen(GameState *state)
{
    // Mulai render ke texture virtual
    BeginTextureMode(state->Dungeon);
    ClearBackground(DARKGRAY);
    
    // Draw loading text
    int textWidth = MeasureText(state->loadingText, 20);
    DrawText(state->loadingText, GameScreenWidth/2 - textWidth/2, GameScreenHeight/2 - 20, 20, WHITE);
    
    // Draw progress bar background
    DrawRectangle(GameScreenWidth/2 - 150, GameScreenHeight/2 + 20, 300, 20, DARKGRAY);
    
    // Draw progress bar fill
    DrawRectangle(GameScreenWidth/2 - 150, GameScreenHeight/2 + 20, (int)(state->loadingProgress * 3), 20, GREEN);
    
    // Draw progress percentage
    char progressText[10];
    sprintf(progressText, "%d%%", (int)state->loadingProgress);
    int progressTextWidth = MeasureText(progressText, 20);
    DrawText(progressText, GameScreenWidth/2 - progressTextWidth/2, GameScreenHeight/2 + 50, 20, WHITE);
    
    EndTextureMode();
}

/**
 * @brief IsLoadingComplete()
 * Memeriksa apakah loading sudah selesai.
 * @param state GameState pointer
 * @return true jika loading selesai, false jika belum
 */
bool IsLoadingComplete(GameState *state)
{
    return state->loadingComplete;
}