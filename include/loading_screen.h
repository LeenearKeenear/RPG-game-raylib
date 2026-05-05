#pragma once

/**
 * @file loading_screen.h
 * @brief Loading Screen Module
 * 
 * Handle the loading screen display and asset loading sequence.
 * Shows progress while loading game assets before transitioning to gameplay.
 */

#include "../lib/raylib/include/raylib.h"
#include "screen.h"
#include "animation.h"

/*==============================================================================
 * Loading Screen Functions
 *==============================================================================*/

/**
 * @brief Initialize loading screen state
 * @param state Pointer to GameState
 */
void InitLoadingScreen(GameState *state);

/**
 * @brief Update loading screen logic
 * @param state Pointer to GameState
 */
void UpdateLoadingScreen(GameState *state);

/**
 * @brief Render loading screen to virtual screen
 * @param state Pointer to GameState
 */
void RenderLoadingScreen(GameState *state);

/**
 * @brief Check if loading is complete
 * @param state Pointer to GameState
 * @return true if loading is complete, false otherwise
 */
bool IsLoadingComplete(GameState *state);