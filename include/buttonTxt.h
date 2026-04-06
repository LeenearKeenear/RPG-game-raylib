#pragma once

#include "../lib/raylib/include/raylib.h"
#include <string>

class buttonTxt 
{
    public:
        buttonTxt(const char* text, int posX, int posY, int fontSize, Color color);
        ~buttonTxt();

        void Draw();
        
        bool isClicked(Vector2 mousePosition, bool mouseClicked) const;

    private:
        const char* text;
        int posX;
        int posY;
        int fontSize;
        Color textColor;
        int textWidth;
};