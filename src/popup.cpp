#include "../include/popup.h"
#include "../include/screen.h"

Popup::Popup() : active(false), message(nullptr), buttonText(nullptr), 
    hoverAmount(1.0F), position({0, 0}), width(0), height(0)
{
}

Popup::Popup(const char* message, const char* buttonText, float hoverAmount) 
    : active(false), message(message), buttonText(buttonText), hoverAmount(hoverAmount),
      position({0, 0}), width(0), height(0)
{
    CalculateDimensions();
}

Popup::~Popup()
{
}

void Popup::Show()
{
    active = true;
}

void Popup::Hide()
{
    active = false;
}

bool Popup::IsActive() const
{
    return active;
}

void Popup::Update(Vector2 mousePosition, bool mouseClicked)
{
    if (!active) return;
    if (okButton.isClicked(mousePosition, mouseClicked)) {
        Hide();
    }
}

void Popup::CalculateDimensions()
{
    const int paddingX = 40;
    const int paddingY = 30;
    const int textPadding = 20;
    const int maxWidth = static_cast<int>(GameScreenWidth * 0.6F);
    const int fontSize = 30;

    int textWidth = MeasureText(message, fontSize);
    int buttonWidth = MeasureText(buttonText, fontSize);

    width = textWidth + paddingX * 2;
    if (buttonWidth + paddingX * 2 > width) {
        width = buttonWidth + paddingX * 2;
    }
    if (width > maxWidth) {
        width = maxWidth;
    }

    height = fontSize + (fontSize * 2) + paddingY * 3 + textPadding;

    position.x = (GameScreenWidth - width) / 2.0F;
    position.y = (GameScreenHeight - height) / 2.0F;

    backgroundRect = {position.x, position.y, static_cast<float>(width), static_cast<float>(height)};

    // Calculate button position: centered horizontally, near bottom of popup
    int buttonX = static_cast<int>(position.x + (width - buttonWidth) / 2.0F);
    int buttonY = static_cast<int>(position.y + height - paddingY - fontSize);

    // Debug logging for button position
    TraceLog(LOG_DEBUG, "Popup Debug: width=%d, height=%d", width, height);
    TraceLog(LOG_DEBUG, "Popup Debug: position=(%.1f, %.1f)", position.x, position.y);
    TraceLog(LOG_DEBUG, "Popup Debug: buttonX=%d, buttonY=%d, buttonWidth=%d", buttonX, buttonY, buttonWidth);

    okButton = buttonTxt(buttonText, buttonX, buttonY, fontSize, WHITE, hoverAmount);
}

void Popup::Draw(Vector2 mousePosition)
{
    if (!active) return;

    Color overlayColor = {
        0,
        0,
        0,
        static_cast<unsigned char>(180 * hoverAmount)
    };

    Rectangle fullScreen = {0, 0, static_cast<float>(GameScreenWidth), static_cast<float>(GameScreenHeight)};
    DrawRectangleRec(fullScreen, overlayColor);

    unsigned char bgAlpha = static_cast<unsigned char>(100 * hoverAmount);
    Color bgColor = {50, 50, 50, bgAlpha};
    DrawRectangleRec(backgroundRect, bgColor);

    int textWidth = MeasureText(message, 30);
    int textX = static_cast<int>(position.x + (width - textWidth) / 2.0F);
    int textY = static_cast<int>(position.y + 30);

    DrawText(message, textX, textY, 30, WHITE);

    okButton.Draw(mousePosition);
}
