#pragma once

/**
 * @file popup.h
 * @brief Popup Dialog System Module
 *
 * Handle popup dialog dengan pesan dan tombol OK.
 * Biasanya dipake buat notifikasi, error, atau konfirmasi sederhana.
 */

#include "../lib/raylib/include/raylib.h"
#include "buttonTxt.h"

/*==============================================================================
 * Popup Class
 *==============================================================================*/

/**
 * @brief Class buat handle popup dialog
 *
 * Popup sederhana dengan:
 * - Pesan teks (bisa multi-line)
 * - Tombol OK buat nutup popup
 * - Background semi-transparan di belakangnya
 *
 * Biasanya dipake buat:
 * - Notifikasi error (misal "Gak bisa load save file")
 * - Konfirmasi sederhana
 * - Informasi ke player
 */
class Popup
{
public:
    /**
     * @brief Constructor default - bikin popup kosong
     * @note Perlu di-set message dan buttonText pake constructor lain
     */
    Popup();

    /**
     * @brief Constructor dengan pesan dan tombol
     * @param message Teks pesan yang ditampilin di popup
     * @param buttonText Teks yang muncul di tombol OK
     * @param hoverAmount Intensitas efek gelap pas hover tombol (default 1.0 = gak ada perubahan)
     *                    - 0.0 = item item (hitam total)
     *                    - 1.0 = gak ada perubahan
     *                    - <1.0 = makin gelap pas hover
     */
    Popup(const char *message, const char *buttonText, float hoverAmount = 1.0F);

    /** @brief Destructor */
    ~Popup();

    /**
     * @brief Tampilin popup ke layar
     * @note Set active = true
     */
    void Show();

    /**
     * @brief Sembunyiin popup dari layar
     * @note Set active = false
     */
    void Hide();

    /**
     * @brief Cek apakah popup lagi aktif/tampil
     * @return true kalo popup muncul, false kalo gak
     */
    bool IsActive() const;

    /**
     * @brief Update logic popup (handle input & tombol OK)
     * @param mousePosition Posisi mouse saat ini
     * @param mouseClicked Apakah tombol mouse diteken
     * @note Kalo tombol OK diklik, otomatis Hide()
     */
    void Update(Vector2 mousePosition, bool mouseClicked);

    /**
     * @brief Render popup ke layar
     * @param mousePosition Posisi mouse saat ini (buat efek hover tombol)
     * @note Cuma render kalo active == true
     */
    void Draw(Vector2 mousePosition);

private:
    /**
     * @brief Hitung dimensi popup berdasarkan teks pesan
     * @note Nentuin width, height, dan posisi biar centered di layar
     */
    void CalculateDimensions();

    /*==========================================================================
     * Private Members
     *==========================================================================*/

    bool active;            /**< Flag apakah popup lagi tampil */
    const char *message;    /**< Teks pesan yang ditampilin */
    const char *buttonText; /**< Teks tombol OK */
    buttonTxt okButton;     /**< Tombol OK buat nutup popup */
    float hoverAmount;      /**< Intensitas efek hover tombol (0.0 - 1.0) */

    Vector2 position;         /**< Posisi popup di layar (centered) */
    int width;                /**< Lebar popup dalam pixel */
    int height;               /**< Tinggi popup dalam pixel */
    Rectangle backgroundRect; /**< Rectangle buat background popup */
};