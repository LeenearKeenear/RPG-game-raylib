#include "../include/buttonTxt.h"

// default constructor
buttonTxt::buttonTxt() : text(nullptr), posX(0), posY(0), fontSize(0), textColor(BLANK), textWidth(0)
{
}

// constructor
// hoverAmount: 0.0 = black, 1.0 = no darkening, <1.0 = darker on hover
buttonTxt::buttonTxt(const char* text, int posX, int posY, int fontSize, Color color, float hoverAmount) {
    this->text = text;
    this->posX = posX;
    this->posY = posY;
    this->fontSize = fontSize;
    this->textColor = color;
    this->textWidth = MeasureText(text, fontSize);
    this->hoverAmount = hoverAmount;
}

// destructor
// Fonts don't need to be deleted
buttonTxt::~buttonTxt()
{    
}

// draw
void buttonTxt::Draw(Vector2 mousePosition) {
    Color currentColor = textColor;
    if (isHovered(mousePosition)) {
        currentColor = (Color) {
            static_cast<unsigned char>(textColor.r * hoverAmount),
            static_cast<unsigned char>(textColor.g * hoverAmount),
            static_cast<unsigned char>(textColor.b * hoverAmount),
            textColor.a
        };
    }

    DrawText(text, posX, posY, fontSize, currentColor);
}

// isClicked
bool buttonTxt::isClicked(Vector2 mousePosition, bool mouseClicked) const
{
    Rectangle textBounds = {
        static_cast<float>(posX),
        static_cast<float>(posY),
        static_cast<float>(textWidth),
        static_cast<float>(fontSize),
    };

    return CheckCollisionPointRec(mousePosition, textBounds) && mouseClicked;
}

// isHovered
bool buttonTxt::isHovered(Vector2 mousePosition) const
{
    Rectangle textBounds = {
        static_cast<float>(posX),
        static_cast<float>(posY),
        static_cast<float>(textWidth),
        static_cast<float>(fontSize),
    };

    return CheckCollisionPointRec(mousePosition, textBounds);
}