#pragma once

/**
 * @file saveLoadScreen.h
 * @brief Menu Simpan/Muat Game Module
 *
 * Header ini mendeklarasikan kelas SaveLoadScreen untuk menangani
 * UI menu simpan dan muat game. Mengikuti pola yang sama dengan
 * OptionsScreen.
 */

#include "../lib/raylib/include/raylib.h"
#include "button.h"
#include "screen.h"

/*==============================================================================
 * SaveLoadScreen Class
 *==============================================================================*/

/**
 * @class SaveLoadScreen
 * @brief Class untuk menangani layar Save/Load
 *
 * Menyediakan antarmuka untuk menyimpan dan memuat state game.
 * Di-render di atas game screen dengan latar belakang gelap.
 * Menggunakan tombol berbasis teks untuk navigasi.
 */
class SaveLoadScreen {
public:
    /**
     * @brief Constructor
     *
     * Menginisialisasi semua member dan tombol navigasi.
     */
    SaveLoadScreen();

    /**
     * @brief Destructor
     */
    ~SaveLoadScreen();

    /**
     * @brief Menampilkan layar save/load
     */
    void Show();

    /**
     * @brief Menyembunyikan layar save/load
     */
    void Hide();

    /** @brief Cek apakah layar save/load sedang aktif */
    bool IsActive() const;

    /**
     * @brief Memperbarui handling input
     * @param state Pointer ke GameState
     * @param mousePosition Posisi mouse saat ini
     * @param mouseClicked Status klik mouse
     */
    void Update(GameState* state, Vector2 mousePosition, bool mouseClicked);

    /**
     * @brief Me-render layar save/load
     * @param mousePosition Posisi mouse untuk efek hover
     */
    void Draw(Vector2 mousePosition);

    /** @brief Set layar tujuan saat tombol BACK diklik */
    void SetReturnScreen(ScreenState screen);

private:
    /**
     * @brief Menghitung dimensi dan posisi elemen UI
     */
    void CalculateDimensions();

    /// Status aktif layar save/load
    bool active;

    /// Flag apakah texture sudah dimuat
    bool texturesLoaded;

    /// Layar tujuan saat tombol BACK diklik
    ScreenState returnScreen;

    /// Tombol BACK untuk kembali ke layar sebelumnya
    buttonTxt backButton;

    /// Lebar area save/load
    int width;

    /// Tinggi area save/load
    int height;

    /// Posisi X awal area save/load
    int startX;

    /// Posisi Y awal area save/load
    int startY;

    /// Rectangle background area save/load
    Rectangle backgroundRect;

    /// Texture background
    Texture2D bgTexture;
};
