// #include <iostream>
#include "../include/button_img.h"

/**
 * Konstruktor Button
 *
 * Membuat sebuah objek Button dengan membaca gambar dari file path yang diberikan,
 * lalu mengubah ukuran gambar sesuai dengan skala yang diberikan.
 * Setelah itu, gambar akan di-load ke dalam texture dan posisi akan di-set.
 *
 * @param texturePath path file gambar yang akan di-load
 * @param imagePosition posisi awal gambar yang akan di-set
 * @param scale skala untuk mengubah ukuran gambar
 */
buttonImage::buttonImage(const char* texturePath, Vector2 imagePosition, float scale)
{
    // Instant texture
    // texture = LoadTexture(texturePath);

    // Resize, then load
    Image image = LoadImage(texturePath);
    int originalWidth = image.width;
    int originalHeight = image.height;

    int newWidth = static_cast<int>(originalWidth * scale);
    int newHeight = static_cast<int>(originalHeight * scale);

    ImageResize(&image, newWidth, newHeight);
    texture = LoadTextureFromImage(image);
    // Unload image from memory
    UnloadImage(image);

    // Set position
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

/**
 * Menggambar tombol pada layar di posisi saat ini.
 * @remarks Tombol digambar dengan warna WHITE.
 */
void buttonImage::Draw() 
{
    DrawTextureV(texture, position, WHITE);
}

/**
 * Memeriksa apakah tombol diklik oleh mouse.
 * @param mousePosition posisi mouse saat ini
 * @param mouseClicked true jika tombol diklik
 * @return true jika tombol diklik oleh mouse
 */
bool buttonImage::isClicked(Vector2 mousePosition, bool mouseClicked)
{
    return CheckCollisionPointRec(mousePosition, {position.x, position.y, static_cast<float>(texture.width), static_cast<float>(texture.height)}) && mouseClicked; 
}