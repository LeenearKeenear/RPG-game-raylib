#pragma once

#include "../lib/raylib/include/raylib.h"
#include "buttonTxt.h"

class Popup 
{
public:
    Popup();
    Popup(const char* message, const char* buttonText, float hoverAmount = 1.0F);
    ~Popup();

    void Show();
    void Hide();
    bool IsActive() const;

    void Draw(Vector2 mousePosition);

private:
    void CalculateDimensions();

    bool active;
    const char* message;
    const char* buttonText;
    buttonTxt okButton;
    float hoverAmount;

    Vector2 position;
    int width;
    int height;
    Rectangle backgroundRect;
};
