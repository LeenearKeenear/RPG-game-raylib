#pragma once

#include "../lib/raylib/include/raylib.h"
#include "buttonTxt.h"

class Popup 
{
public:
    Popup();
    Popup(const char* message, const char* buttonText, float hoverAmount = 1.0F);
    Popup(const char* message, const char* confirmText, const char* cancelText, float hoverAmount);
    ~Popup();

    void Show();
    void Hide();
    bool IsActive() const;
    bool IsConfirmClicked() const;

    void Update(Vector2 mousePosition, bool mouseClicked);
    void Draw(Vector2 mousePosition);

private:
    void CalculateDimensions();

    bool active;
    bool hasCancelButton;
    const char* message;
    const char* buttonText;
    const char* cancelText;
    buttonTxt okButton;
    buttonTxt cancelButton;
    float hoverAmount;
    bool confirmClicked;

    Vector2 position;
    int width;
    int height;
    Rectangle backgroundRect;
};
