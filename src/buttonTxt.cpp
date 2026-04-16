/**
 * @file buttonTxt.cpp
 * @brief Implementasi dari Text-based UI Button System
 *
 * Implementasi dari class buttonTxt yang dideklarasikan di buttonTxt.h
 * Handle rendering teks sebagai tombol dengan efek hover.
 */

#include "../include/buttonTxt.h"

/*==============================================================================
 * Constructors & Destructor
 *==============================================================================*/

/**
 * @brief Default constructor - bikin tombol teks kosong
 * @note Semua properti di-set ke nilai default/null
 */
buttonTxt::buttonTxt() : text(nullptr), posX(0), posY(0), fontSize(0), textColor(BLANK), textWidth(0)
{
}

/**
 * @brief Constructor dengan parameter lengkap
 * @param text Teks yang bakal ditampilin
 * @param posX Posisi X di layar
 * @param posY Posisi Y di layar
 * @param fontSize Ukuran font dalam pixel
 * @param color Warna teks normal
 * @param hoverAmount: 0.0 = hitam, 1.0 = gak ada perubahan, <1.0 = lebih gelap pas hover
 */
buttonTxt::buttonTxt(const char *text, int posX, int posY, int fontSize, Color color, float hoverAmount)
{
    this->text = text;
    this->posX = posX;
    this->posY = posY;
    this->fontSize = fontSize;
    this->textColor = color;
    this->textWidth = MeasureText(text, fontSize); // hitung lebar teks pake raylib
    this->hoverAmount = hoverAmount;
}

/**
 * @brief Destructor
 * @note Gak perlu cleanup khusus karena teks adalah string literal / dikelola external
 */
buttonTxt::~buttonTxt()
{
}

/*==============================================================================
 * Rendering
 *==============================================================================*/

/**
 * @brief Render tombol teks ke layar
 * @param mousePosition Posisi mouse saat ini (buat deteksi hover)
 * @note Kalo mouse hover, warna teks jadi lebih gelap sesuai hoverAmount
 */
void buttonTxt::Draw(Vector2 mousePosition)
{
    Color currentColor = textColor;

    // Efek hover: kalo mouse di atas teks, redupkan warna
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

/*==============================================================================
 * State Checks
 *==============================================================================*/

/**
 * @brief Memeriksa apakah tombol diklik oleh mouse.
 * @param mousePosition posisi mouse saat ini
 * @param mouseClicked true jika tombol mouse ditekan
 * @return true kalo mouse di area teks DAN mouseClicked true
 */
bool buttonTxt::isClicked(Vector2 mousePosition, bool mouseClicked) const
{
    // Bikin rectangle berdasarkan posisi dan ukuran teks
    Rectangle textBounds = {
        static_cast<float>(posX),
        static_cast<float>(posY),
        static_cast<float>(textWidth),
        static_cast<float>(fontSize),
    };

    return CheckCollisionPointRec(mousePosition, textBounds) && mouseClicked;
}

/**
 * @brief Memeriksa apakah mouse terletak di atas tombol.
 * @param mousePosition posisi mouse saat ini
 * @return true kalo mouse di area teks
 */
bool buttonTxt::isHovered(Vector2 mousePosition) const
{
    // Bikin rectangle berdasarkan posisi dan ukuran teks
    Rectangle textBounds = {
        static_cast<float>(posX),
        static_cast<float>(posY),
        static_cast<float>(textWidth),
        static_cast<float>(fontSize),
    };

    return CheckCollisionPointRec(mousePosition, textBounds);
}