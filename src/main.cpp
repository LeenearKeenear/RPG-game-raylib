#include "../include/screen.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../include/mainMenu.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"

int main()
{
    // Initialize the game window and rendering system
    // Creates a virtual screen (1280x720) that scales to fit the actual window
    GameState state = InitScreen();

    // Initialize the map data (load tiles, parse JSON, etc.)
    InitDrawMap(&state);

    // Initialize player position and other game entities
    InitAll();

    // Initialize main menu UI elements (buttons, etc.)
    InitMainMenu(&state);

    // Main game loop - runs every frame until window is closed
    while (!WindowShouldClose())
    {
        // MAIN_MENU state - displays the start screen with buttons
        if (state.currentScreen == MAIN_MENU)
        {
            // UpdateGame handles window resize events and updates ScaleMultiplier
            // This ensures the menu scales correctly when window size changes
            UpdateGame(&state);
            // Handle button clicks and transitions
            UpdateMainMenu(&state);
            // Draw buttons to the virtual screen (1280x720 RenderTexture)
            RenderMainMenuToVirtualScreen(&state);
            // Draw the virtual screen to the window, scaling to fit while maintaining aspect ratio
            DrawRenderWindows(&state);
        }
        // PLAY state - the actual gameplay
        else if (state.currentScreen == PLAY)
        {
            // Update window/scale info and game logic
            UpdateGame(&state);
            // Update player movement and actions
            UpdatePlayer(&state);
            // Render game world to virtual screen
            DrawRenderTexture(&state);
            // Draw virtual screen to window with proper scaling
            DrawRenderWindows(&state);
        }
    }

    // Clean up resources (textures, maps, window, etc.)
    GameShutDown(&state);
    return 0;
}