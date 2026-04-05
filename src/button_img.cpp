// #include <iostream>
#include "../include/button_img.h"

Button::Button(const char* texturePath, Vector2 position, float scale)
{
    texture = LoadTexture(texturePath);
}

Button::~Button()
{
    UnloadTexture(texture);
}

void Button::Draw()
{
    DrawTextureV(texture, position, WHITE);
}