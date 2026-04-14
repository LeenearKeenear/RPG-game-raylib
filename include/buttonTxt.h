#pragma once

#include "../lib/raylib/include/raylib.h"
#include <string>

class buttonTxt 
{
    public:
        buttonTxt();
        buttonTxt(const char* text, int posX, int posY, int fontSize, Color color, float hoverAmount = 1.0F);
        // hoverAmount: 0.0 = black, 1.0 = no darkening, <1.0 = darker on hover
        
        ~buttonTxt();

        void Draw(Vector2 mousePosition);

        // state checks
        [[nodiscard]] bool isClicked(Vector2 mousePosition, bool mouseClicked) const;
        [[nodiscard]] bool isHovered(Vector2 mousePosition) const;

    private:
        const char* text;
        int posX;
        int posY;
        int fontSize;
        Color textColor;
        int textWidth;
        float hoverAmount;
};