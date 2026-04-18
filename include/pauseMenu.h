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
    
    bool active;                                    /**< Flag apakah menu aktif/tampil */
    std::array<buttonTxt, 6> buttons;              /**< Array tombol-tombol menu (6 buah) */
    std::array<const char*, 6> buttonTexts;        /**< Teks buat masing-masing tombol */

    Vector2 position;                               /**< Posisi menu di layar (centered) */
    int width;                                      /**< Lebar menu dalam pixel */
    int height;                                     /**< Tinggi menu dalam pixel */
    Rectangle backgroundRect;                       /**< Rectangle buat background menu */
};