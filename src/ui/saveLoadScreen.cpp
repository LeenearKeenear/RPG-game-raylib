/**
 * @file saveLoadScreen.cpp
 * @brief Menu Simpan/Muat Game Module
 *
 * Implementasi kelas SaveLoadScreen untuk menangani
 * UI menu simpan dan muat game.
 */

#include "../../include/ui/saveLoadScreen.h"

/*==============================================================================
 * Constructor / Destructor
 *==============================================================================*/

/**
 * @brief Constructor
 *
 * Menginisialisasi semua member dan tombol navigasi.
 * Menggunakan buttonTxt (berbasis teks) untuk tombol BACK.
 */
SaveLoadScreen::SaveLoadScreen()
    : active(false)
    , texturesLoaded(false)
    , returnScreen(PLAY)
    , width(0)
    , height(0)
    , startX(0)
    , startY(0)
    , bgTexture({0})
{
}

/**
 * @brief Destructor
 *
 * Membersihkan resource texture background jika sudah dimuat.
 */
SaveLoadScreen::~SaveLoadScreen()
{
    if (bgTexture.id != 0) {
        UnloadTexture(bgTexture);
    }
}

/*==============================================================================
 * Public Methods
 *==============================================================================*/

/**
 * @brief Menampilkan layar save/load
 *
 * Mengaktifkan flag active dan menghitung ulang dimensi UI.
 * Texture background akan dimuat saat Show() pertama kali dipanggil.
 */
void SaveLoadScreen::Show()
{
    active = true;
    if (!texturesLoaded) {
        // Texture akan dimuat di task mendatang jika diperlukan
        texturesLoaded = true;
    }
    CalculateDimensions();
}

/**
 * @brief Menyembunyikan layar save/load
 */
void SaveLoadScreen::Hide()
{
    active = false;
}

/**
 * @brief Memeriksa apakah layar save/load sedang aktif
 * @return true jika aktif, false jika tidak
 */
bool SaveLoadScreen::IsActive() const
{
    return active;
}

/**
 * @brief Mengatur layar kembali saat BACK diklik
 * @param screen Layar tujuan
 */
void SaveLoadScreen::SetReturnScreen(ScreenState screen)
{
    returnScreen = screen;
}

/*==============================================================================
 * Update & Draw
 *==============================================================================*/

/**
 * @brief Memperbarui handling input
 * @param state Pointer ke GameState
 * @param mousePosition Posisi mouse saat ini
 * @param mouseClicked Status klik mouse
 *
 * Memeriksa tombol BACK. Jika diklik, menyembunyikan layar
 * dan mengembalikan state ke returnScreen.
 */
void SaveLoadScreen::Update(GameState* state, Vector2 mousePosition, bool mouseClicked)
{
    if (!active) {
        return;
    }

    if (backButton.isClicked(mousePosition, mouseClicked)) {
        active = false;
        state->currentScreen = returnScreen;
        return;
    }
}

/**
 * @brief Me-render layar save/load
 * @param mousePosition Posisi mouse untuk efek hover
 *
 * Menggambar background menu dan tombol BACK.
 * Slot grid rendering akan ditambahkan di task mendatang.
 */
void SaveLoadScreen::Draw(Vector2 mousePosition)
{
    if (!active) {
        return;
    }

    DrawMenuBackground();

    if (bgTexture.id != 0) {
        DrawTexture(bgTexture, startX, startY, WHITE);
    } else {
        Color bgColor = {40, 40, 40, 230};
        DrawRectangleRec(backgroundRect, bgColor);
        DrawRectangleLinesEx(backgroundRect, 2, WHITE);
    }

    backButton.Draw(mousePosition);
}

/*==============================================================================
 * Private Methods
 *==============================================================================*/

/**
 * @brief Menghitung dimensi dan posisi elemen UI
 *
 * Mengatur ukuran panel (600x400), memusatkannya di layar,
 * dan memposisikan tombol BACK di pojok kanan bawah panel.
 */
void SaveLoadScreen::CalculateDimensions()
{
    width = 600;
    height = 400;
    startX = (GameScreenWidth - width) / 2;
    startY = (GameScreenHeight - height) / 2;

    backgroundRect = {
        static_cast<float>(startX),
        static_cast<float>(startY),
        static_cast<float>(width),
        static_cast<float>(height)
    };

    backButton = buttonTxt(
        "BACK",
        startX + width - 100,
        startY + height - 50,
        24,
        WHITE,
        0.7F);
}
