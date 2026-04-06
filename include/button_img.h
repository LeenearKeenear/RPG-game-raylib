#pragma once
#include "../lib/raylib/include/raylib.h"

class buttonImage 
{
    public:
        buttonImage(const char* texturePath, Vector2 position, float scale);
        ~buttonImage();

        void Draw();

        bool isClicked(Vector2 mousePosition, bool mouseClicked);
    private:
        Texture2D texture;
        Vector2 position;
};