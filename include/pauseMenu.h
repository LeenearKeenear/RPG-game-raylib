#pragma once

/**
 * @file pausemenu.h
 * @brief Pause Menu System Module
 * 
 * Handle pause menu UI pas game di-pause.
 * Muncul di tengah layar dengan tombol-tombol:
 * - Resume, Save, Load, Options, Controls, Quit
 */

#include "../lib/raylib/include/raylib.h"
#include "buttonTxt.h"
#include "screen.h"
#include <array>
#include <cstdint>
#include <vector>

/*==============================================================================
 * Video Settings Constants
 *==============================================================================*/

/**
 * @brief Resolution presets untuk video settings
 * @note Setiap resolusi difilter berdasarkan max monitor resolution
 */
struct ResOption {
    int width;
    int height;
    const char* label;
};

/*==============================================================================
 * OptionsScreen Class
 *==============================================================================*/

/**
 * @brief Class for handling the standalone Options screen
 * 
 * Has 3 tabs: Video, Audio, Keybinds
 * Accessed from Main Menu or Pause Menu
 */
class OptionsScreen {
public:
    /** @brief Constructor */
    OptionsScreen();

    /** @brief Destructor */
    ~OptionsScreen();

    /** @brief Show the options screen */
    void Show();

    /** @brief Hide the options screen */
    void Hide();

    /** @brief Check if options screen is active */
    bool IsActive() const;

    /** @brief Update input handling */
    void Update(GameState* state, Vector2 mousePosition, bool mouseClicked);

    /** @brief Render the options screen */
    void Draw(Vector2 mousePosition);

    /** @brief Set return screen (what screen to return to) */
    void SetReturnScreen(ScreenState screen);

private:
    void CalculateDimensions();
    void DrawVideoTab(Vector2 mousePosition, bool showFPS);
    void DrawAudioTab(Vector2 mousePosition);
    void DrawKeybindsTab(Vector2 mousePosition);

    bool showFPS;

    bool active;
    ScreenState returnScreen;
    int selectedTab;

    std::array<buttonTxt, 3> tabButtons;
    buttonTxt backButton;

    int width;
    int height;
    int startX;
    int startY;
    Rectangle backgroundRect;

    std::vector<ResOption> resolutionOptions;
    int selectedResolution;
};

/*==============================================================================
 * PauseMenu Class
 *==============================================================================*/

/**
 * @brief Class buat handle pause menu
 * 
 * Menu yang muncul pas player pause game.
 * Render di atas game screen (overlay) dengan background semi-transparan.
 * 
 * Tombol-tombol yang tersedia:
 * - Resume      → lanjutin game
 * - Save        → save game state
 * - Load        → load game state
 * - Options     → buka menu options
 * - Controls    → liat kontrol
 * - Quit        → balik ke main menu
 */
class PauseMenu {
public:
    /** @brief Constructor - inisialisasi menu dan semua tombol */
    PauseMenu();
    
    /** @brief Destructor - bersihin resource kalo perlu */
    ~PauseMenu();

    /**
     * @brief Tampilin pause menu
     * @note Set active = true, game logic harus berhenti
     */
    void Show();
    
    /**
     * @brief Sembunyiin pause menu
     * @note Set active = false, game logic lanjut lagi
     */
    void Hide();
    
    /**
     * @brief Cek apakah pause menu lagi aktif
     * @return true kalo menu muncul, false kalo gak
     */
    bool IsActive() const;

    /**
     * @brief Update logic pause menu (handle input & button clicks)
     * @param state GameState pointer - buat akses ke data game
     * @param mousePosition Posisi mouse saat ini
     * @param mouseClicked Apakah tombol mouse diteken
     */
    void Update(GameState* state, Vector2 mousePosition, bool mouseClicked);
    
    /**
     * @brief Render pause menu ke layar
     * @param mousePosition Posisi mouse saat ini (buat efek hover)
     */
    void Draw(Vector2 mousePosition);

private:
    /**
     * @brief Hitung dimensi menu berdasarkan layar
     * @note Nentuin width, height, dan posisi biar centered
     */
    void CalculateDimensions();
    
    /**
     * @brief Handle klik pada tombol berdasarkan index
     * @param buttonIndex Index tombol yang diklik (0-5)
     * @param state GameState pointer - buat eksekusi aksi
     */
    void HandleButtonClick(int buttonIndex, GameState* state);

    /*==========================================================================
     * Private Members
     *==========================================================================*/
    
    bool active;
    std::array<buttonTxt, 6> buttons;
    std::array<const char*, 6> buttonTexts;

    Vector2 position;
    int width;
    int height;
    Rectangle backgroundRect;
};