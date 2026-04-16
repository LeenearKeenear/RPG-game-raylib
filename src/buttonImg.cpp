/**
 * @file buttonImg.cpp
 * @brief Implementasi dari Texture-based UI Button System
 *
 * Implementasi dari class buttonImage yang dideklarasikan di buttonImg.h
 * Handle loading texture, resize dengan scale, rendering dengan efek hover.
 */

// #include <iostream>  // (sementara di-comment, gak dipake)
#include "../include/buttonImg.h"

/*==============================================================================
 * Constructor & Destructor
 *==============================================================================*/

/**
 * @brief Konstruktor Button
 *
 * @brief Membuat sebuah objek Button dengan membaca gambar dari file path yang diberikan,
 * lalu mengubah ukuran gambar sesuai dengan skala yang diberikan.
 * Setelah itu, gambar akan di-load ke dalam texture dan posisi akan di-set.
 *
 * @param texturePath path file gambar yang akan di-load
 * @param imagePosition posisi awal gambar yang akan di-set
 * @param scale skala untuk mengubah ukuran gambar
 * @param hoverAmount nilai hoverAmount (0.0 = hitam, 1.0 = normal, <1.0 = lebih gelap)
 *
 */
buttonImage::buttonImage(const char *texturePath, Vector2 imagePosition, float scale, float hoverAmount)
{
    // Instant texture (cara lama, sekarang pake resize dulu)
    // texture = LoadTexture(texturePath);

    // Resize, then load
    // Step 1: Load image dari file
    Image image = LoadImage(texturePath);
    int originalWidth = image.width;
    int originalHeight = image.height;

    // Step 2: Hitung ukuran baru berdasarkan scale
    int newWidth = static_cast<int>(originalWidth * scale);
    int newHeight = static_cast<int>(originalHeight * scale);

    // Step 3: Resize image ke ukuran baru
    ImageResize(&image, newWidth, newHeight);

    // Step 4: Simpan hoverAmount untuk efek gelap pas hover
    this->hoverAmount = hoverAmount;

    // Step 5: Konversi image ke texture (bisa langsung dipake raylib)
    texture = LoadTextureFromImage(image);

    // Step 6: Unload image dari memory (gak perlu lagi setelah jadi texture)
    UnloadImage(image);

    // Step 7: Set posisi tombol di layar
    position = imagePosition;
}

/**
 * Dekonstruktor kelas Button. Menghapus tekstur dari memori.
 * @remarks Metode ini dipanggil secara otomatis ketika instance kelas Button dihancurkan.
 */
buttonImage::~buttonImage()
{
    // Unload the texture from memory
    UnloadTexture(texture);
}

/*==============================================================================
 * Rendering
 *==============================================================================*/

/**
 * Menggambar tombol pada layar di posisi saat ini.
 * @remarks Tombol digambar dengan warna WHITE, kalo hover warnanya jadi lebih gelap sesuai hoverAmount.
 */
void buttonImage::Draw(Vector2 mousePosition)
{
    Color currentColor = WHITE;

    // Efek hover: kalo mouse di atas tombol, redupkan warna
    if (isHovered(mousePosition))
    {
        currentColor = (Color){
            static_cast<unsigned char>(currentColor.r * hoverAmount),
            static_cast<unsigned char>(currentColor.g * hoverAmount),
            static_cast<unsigned char>(currentColor.b * hoverAmount),
            currentColor.a};
    }

    DrawTextureV(texture, position, currentColor);
}

/*==============================================================================
 * State Checks
 *==============================================================================*/

/**
 * @brief Memeriksa apakah tombol diklik oleh mouse.
 *
 * @param mousePosition posisi mouse saat ini
 * @param mouseClicked true jika tombol mouse ditekan
 * @return true jika mouse di area tombol DAN mouseClicked true
 */
bool buttonImage::isClicked(Vector2 mousePosition, bool mouseClicked) const
{
    // Cek collision antara posisi mouse dengan rectangle tombol
    // Pake CheckCollisionPointRec dari raylib
    return CheckCollisionPointRec(mousePosition, {position.x, position.y, static_cast<float>(texture.width), static_cast<float>(texture.height)}) && mouseClicked;
}

/**
 * @brief Memeriksa apakah cursor berada di atas tombol.
 *
 * @param mousePosition posisi mouse saat ini
 * @return true kalo mouse di area tombol
 */
bool buttonImage::isHovered(Vector2 mousePosition) const
{
    // Cek collision antara posisi mouse dengan rectangle tombol
    return CheckCollisionPointRec(mousePosition, {position.x, position.y, static_cast<float>(texture.width), static_cast<float>(texture.height)});
}