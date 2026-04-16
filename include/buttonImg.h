#pragma once

/**
 * @file buttonimg.h
 * @brief Texture-based UI Button System
 *
 * Nyediain tombol klik berbasis texture.
 * Ada efek hover gelap dan deteksi klik di area texture.
 */

#include "../lib/raylib/include/raylib.h"

/*==============================================================================
 * ButtonImage Class
 *==============================================================================*/

/**
 * @brief Tombol berbasis texture buat UI
 *
 * Render tombol dari PNG dengan efek hover gelap.
 * Pake isClicked() buat deteksi klik dan Draw() buat ngerender.
 */
class buttonImage
{
public:
    /**
     * @brief Constructor buttonImage
     * @param texturePath File path ke gambar PNG tombol
     * @param position Posisi tombol di layar (X, Y) dalam pixel (top-left corner)
     * @param scale Faktor skala texture (1.0 = ukuran asli)
     * @param hoverAmount Intensitas efek gelap pas hover
     *                    - 0.0 = item item (hitam total)
     *                    - 1.0 = gak ada perubahan
     *                    - <1.0 = makin gelap pas hover (contoh 0.7 = 30% lebih gelap)
     */
    buttonImage(const char *texturePath, Vector2 position, float scale, float hoverAmount = 1.0F);

    /** @brief Destructor - bersihin texture yang udah di-load */
    ~buttonImage();

    /**
     * @brief Render tombol ke layar
     * @param mousePosition Posisi kursor mouse saat ini (dari GetMousePosition())
     * @note Otomatis ngasih efek hover gelap kalo mouse di atas tombol
     */
    void Draw(Vector2 mousePosition);

    /*==========================================================================
     * State Checks
     *==========================================================================*/

    /**
     * @brief Cek apakah tombol diklik
     * @param mousePosition Posisi kursor mouse saat ini
     * @param mouseClicked Apakah tombol mouse lagi diteken
     * @return true kalo mouse di area tombol DAN mouseClicked true
     */
    [[nodiscard]] bool isClicked(Vector2 mousePosition, bool mouseClicked) const;

    /**
     * @brief Cek apakah mouse lagi hover di atas tombol
     * @param mousePosition Posisi kursor mouse saat ini
     * @return true kalo posisi mouse ada di dalam area texture tombol
     */
    [[nodiscard]] bool isHovered(Vector2 mousePosition) const;

private:
    Texture2D texture; /**< Texture tombol yang udah di-load */
    Vector2 position;  /**< Posisi tombol di layar (top-left corner) */
    float hoverAmount; /**< Intensitas efek gelap pas hover (0.0 - 1.0) */
};