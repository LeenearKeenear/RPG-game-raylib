#include "../include/buttonTxt.h"

// constructor
buttonTxt::buttonTxt(const char* text, int posX, int posY, int fontSize, Color color) {
    this->text = text;
    this->posX = posX;
    this->posY = posY;
    this->fontSize = fontSize;
    this->textColor = color;
    this->textWidth = MeasureText(text, fontSize);
}

// destructor
// Fonts don't need to be deleted
buttonTxt::~buttonTxt()
{    
}

// draw
void buttonTxt::Draw() {
    DrawText(text, posX, posY, fontSize, textColor);
}

// isClicked
bool buttonTxt::isClicked(Vector2 mousePosition, bool mouseClicked) const
{
    Rectangle textBounds = {
        static_cast<float>(posX),
        static_cast<float>(posY),
        static_cast<float>(textWidth),
        static_cast<float>(fontSize)
    };

    return CheckCollisionPointRec(mousePosition, textBounds) && mouseClicked;
    }