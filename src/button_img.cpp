// #include <iostream>
#include "../include/button_img.h"

// constructor
Button::Button(const char* texturePath, Vector2 imagePosition, float scale)
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

// destructor
Button::~Button()
{
    UnloadTexture(texture);
}

// Draw method
void Button::Draw()
{
    DrawTextureV(texture, position, WHITE);
}