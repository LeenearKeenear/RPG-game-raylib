#pragma once

#include "../lib/raylib/include/raylib.h"

class buttonImage 
{
    public:
        buttonImage(const char* texturePath, Vector2 position, float scale, float hoverAmount = 1.0F);
        // hoverAmount: 0.0 = black, 1.0 = no darkening, <1.0 = darker on hover

        ~buttonImage();

        void Draw(Vector2 mousePosition);

        // state checks
        [[nodiscard]] bool isClicked(Vector2 mousePosition, bool mouseClicked) const;
        [[nodiscard]] bool isHovered(Vector2 mousePosition) const;
    
    private:
        Texture2D texture;
        Vector2 position;
        float hoverAmount;
};