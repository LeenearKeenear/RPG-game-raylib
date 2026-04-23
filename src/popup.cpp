#include <algorithm>

#include "../include/popup.h"
#include "../include/screen.h"

/**
 * @brief Default constructor.
 */
Popup::Popup() : active(false), message(nullptr), buttonText(nullptr), 
    hoverAmount(1.0F), position({0, 0}), width(0), height(0)
{
}

/**
 * @brief Parameterized constructor.
 * @param message Teks yang ditampilkan di popup.
 * @param buttonText Teks pada tombol.
 * @param hoverAmount Nilai pengurangan warna saat hover (0.0 = hitam, 1.0 = normal).
 */
Popup::Popup(const char* message, const char* buttonText, float hoverAmount) 
    : active(false), message(message), buttonText(buttonText), hoverAmount(hoverAmount), position({0, 0}), width(0), height(0)
{
    CalculateDimensions();
}

/**
 * @brief Destructor.
 */
Popup::~Popup()
{
}

/**
 * @brief Show()
 * Tampilkan popup ke layar.
 */
void Popup::Show()
{
    CalculateDimensions();
    active = true;
}

/**
 * @brief Hide()
 * Sembunyikan popup dari layar.
 */
void Popup::Hide()
{
    active = false;
}

/**
 * @brief IsActive()
 * @return true jika popup sedang aktif.
 */
bool Popup::IsActive() const
{
    return active;
}

/**
 * @brief Update()
 * Update state popup - handle klik tombol OK.
 * @param mousePosition Posisi mouse saat ini.
 * @param mouseClicked true jika tombol mouse diklik.
 */
void Popup::Update(Vector2 mousePosition, bool mouseClicked)
{
    if (!active) {
        return;
    }
    if (okButton.isClicked(mousePosition, mouseClicked)) {
        Hide();
    }
}

/**
 * @brief CalculateDimensions()
 * Hitung dimensi popup berdasarkan teks message dan button.
 */
void Popup::CalculateDimensions()
{
    const int paddingX = 40;
    const int paddingY = 30;
    const int textPadding = 20;
    const int maxWidth = static_cast<int>(GameScreenWidth * 0.6F);
    const int fontSize = 30;

    int textWidth = MeasureText(message, fontSize);
    int buttonWidth = MeasureText(buttonText, fontSize);

    width = textWidth + (paddingX * 2);
    width = std::max(buttonWidth + (paddingX * 2), width);
    width = std::min(width, maxWidth);

    height = fontSize + (fontSize * 2) + (paddingY * 3) + textPadding;

    position.x = (GameScreenWidth - width) / 2.0F;
    position.y = (GameScreenHeight - height) / 2.0F;

    backgroundRect = {position.x, position.y, static_cast<float>(width), static_cast<float>(height)};

    int buttonX = static_cast<int>(position.x + ((width - buttonWidth) / 2.0F));
    int buttonY = static_cast<int>(position.y + height - paddingY - fontSize);

    TraceLog(LOG_DEBUG, "Popup Debug: width=%d, height=%d", width, height);
    TraceLog(LOG_DEBUG, "Popup Debug: position=(%.1f, %.1f)", position.x, position.y);
    TraceLog(LOG_DEBUG, "Popup Debug: buttonX=%d, buttonY=%d, buttonWidth=%d", buttonX, buttonY, buttonWidth);

    okButton = buttonTxt(buttonText, buttonX, buttonY, fontSize, WHITE, hoverAmount);
}

/**
 * @brief Draw()
 * Render popup: overlay, background, message, dan button.
 * @param mousePosition Posisi mouse saat ini.
 */
void Popup::Draw(Vector2 mousePosition)
{
    if (!active) { 
        return;
    }

    Color overlayColor = {
        0,
        0,
        0,
        static_cast<unsigned char>(255 * 0.5F)
    };

    Rectangle fullScreen = {0, 0, static_cast<float>(GameScreenWidth), static_cast<float>(GameScreenHeight)};
    DrawRectangleRec(fullScreen, overlayColor);

    Color bgColor = {20, 20, 20, 255};
    DrawRectangleRec(backgroundRect, bgColor);
    DrawRectangleLines(
        static_cast<int>(backgroundRect.x),
        static_cast<int>(backgroundRect.y),
        static_cast<int>(backgroundRect.width),
        static_cast<int>(backgroundRect.height),
        WHITE
    );

    int textWidth = MeasureText(message, 30);
    int textX = static_cast<int>(position.x + ((width - textWidth) / 2.0F));
    int textY = static_cast<int>(position.y + 30);

    DrawText(message, textX, textY, 30, WHITE);

    okButton.Draw(mousePosition);
}
