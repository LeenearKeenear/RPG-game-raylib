#pragma once
#include "../lib/raylib/include/raylib.h"

class Button 
{
    public:
        Button(const char* texturePath, Vector2 position, float scale);
        ~Button();

        void Draw();

    private:
        Texture2D texture;
        Vector2 position;
};