#pragma once

/**
 * @file buttontxt.h
 * @brief Text-based UI Button System
 *
 * Nyediain tombol berbasis teks untuk UI.
 * Ada efek hover gelap dan deteksi klik di area teks.
 */

#include "../lib/raylib/include/raylib.h"
#include <string>

/*==============================================================================
 * ButtonTxt Class
 *==============================================================================*/

/**
 * @brief Tombol berbasis teks buat UI
 *
 * Render tombol dari teks dengan efek hover gelap.
 * Pake isClicked() buat deteksi klik dan Draw() buat ngerender.
 * Lebar teks diitung otomatis pas konstruktor.
 */
class buttonTxt
{
public:
    /**
     * @brief Constructor default - bikin tombol kosong
     */
    buttonTxt() : text(nullptr), posX(0), posY(0), fontSize(0), textColor(BLANK), textWidth(0)
    {
    }

    /**
     * @brief Constructor dengan parameter lengkap
     * @param text Teks yang bakal ditampilin di tombol
     * @param posX Posisi X tombol di layar (pixel)
     * @param posY Posisi Y tombol di layar (pixel)
     * @param fontSize Ukuran font dalam pixel
     * @param color Warna teks normal (saat gak hover)
     * @param hoverAmount Intensitas efek gelap pas hover
     *                    - 0.0 = item item (hitam total)
     *                    - 1.0 = gak ada perubahan
     *                    - <1.0 = makin gelap pas hover (contoh 0.7 = 30% lebih gelap)
     */
    buttonTxt(const char *text, int posX, int posY, int fontSize, Color color, float hoverAmount = 1.0F)
    {
        this->text = text;
        this->posX = posX;
        this->posY = posY;
        this->fontSize = fontSize;
        this->textColor = color;
        this->textWidth = MeasureText(text, fontSize);
        this->hoverAmount = hoverAmount;
    }

    /** @brief Destructor */
    ~buttonTxt()
    {
    }

    /**
     * @brief Render tombol teks ke layar
     * @param mousePosition Posisi kursor mouse saat ini (dari GetMousePosition())
     * @note Otomatis ngasih efek hover gelap kalo mouse di atas teks
     *       Efek hover dilakukan dengan mengurangi komponen warna RGB
     */
    void Draw(Vector2 mousePosition)
    {
        Color currentColor = textColor;
        if (isHovered(mousePosition))
        {
            currentColor = (Color){
                static_cast<unsigned char>(textColor.r * hoverAmount),
                static_cast<unsigned char>(textColor.g * hoverAmount),
                static_cast<unsigned char>(textColor.b * hoverAmount),
                textColor.a};
        }
        DrawText(text, posX, posY, fontSize, currentColor);
    }

    /*==========================================================================
     * State Checks
     *==========================================================================*/

    /**
     * @brief Cek apakah tombol diklik
     * @param mousePosition Posisi kursor mouse saat ini
     * @param mouseClicked Apakah tombol mouse lagi diteken
     * @return true kalo mouse di area teks tombol DAN mouseClicked true
     */
    [[nodiscard]] bool isClicked(Vector2 mousePosition, bool mouseClicked) const
    {
        Rectangle textBounds = {
            static_cast<float>(posX),
            static_cast<float>(posY),
            static_cast<float>(textWidth),
            static_cast<float>(fontSize),
        };
        return CheckCollisionPointRec(mousePosition, textBounds) && mouseClicked;
    }

    /**
     * @brief Cek apakah mouse lagi hover di atas tombol
     * @param mousePosition Posisi kursor mouse saat ini
     * @return true kalo posisi mouse ada di dalam area teks tombol
     * @note Area deteksi diitung berdasarkan lebar teks yang udah diukur
     */
    [[nodiscard]] bool isHovered(Vector2 mousePosition) const
    {
        Rectangle textBounds = {
            static_cast<float>(posX),
            static_cast<float>(posY),
            static_cast<float>(textWidth),
            static_cast<float>(fontSize),
        };
        return CheckCollisionPointRec(mousePosition, textBounds);
    }

private:
    /*==========================================================================
     * Private Members
     *==========================================================================*/

    const char *text;  /**< Teks yang ditampilin di tombol */
    int posX;          /**< Posisi X tombol di layar (pixel) */
    int posY;          /**< Posisi Y tombol di layar (pixel) */
    int fontSize;      /**< Ukuran font dalam pixel */
    Color textColor;   /**< Warna teks normal (saat gak hover) */
    int textWidth;     /**< Lebar teks dalam pixel (dihitung pas konstruktor pake MeasureText()) */
    float hoverAmount; /**< Intensitas efek gelap pas hover (0.0 - 1.0) */
};