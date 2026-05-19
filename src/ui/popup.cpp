#include <algorithm>

#include "popup.h"
#include "screen.h"

/**
 * @brief Default constructor.
 */
Popup::Popup() : active(false), hasCancelButton(false), message(nullptr), buttonText(nullptr),
    cancelText(nullptr), hoverAmount(1.0F), confirmClicked(false), position({0, 0}), width(0), height(0)
{
}

/**
 * @brief Parameterized constructor.
 * @param message Teks yang ditampilkan di popup.
 * @param buttonText Teks pada tombol.
 * @param hoverAmount Nilai pengurangan warna saat hover (0.0 = hitam, 1.0 = normal).
 */
Popup::Popup(const char* message, const char* buttonText, float hoverAmount) 
    : active(false), hasCancelButton(false), message(message), buttonText(buttonText),
      cancelText(nullptr), hoverAmount(hoverAmount), confirmClicked(false), position({0, 0}), width(0), height(0)
{
    CalculateDimensions();
}

/**
 * @brief Two-button constructor.
 * @param message Teks yang ditampilkan di popup.
 * @param confirmText Teks pada tombol konfirmasi.
 * @param cancelText Teks pada tombol batal.
 * @param hoverAmount Nilai pengurangan warna saat hover (0.0 = hitam, 1.0 = normal).
 */
Popup::Popup(const char* message, const char* confirmText, const char* cancelText, float hoverAmount)
    : active(false), hasCancelButton(true), message(message), buttonText(confirmText),
      cancelText(cancelText), hoverAmount(hoverAmount), confirmClicked(false),
      position({0, 0}), width(0), height(0)
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
    confirmClicked = false;
    CalculateDimensions();
    active = true;
}

/**
 * @brief Hide()
 * Sembunyikan popup dari layar.
 */
void Popup::Hide()
{
    confirmClicked = false;
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
 * @brief IsConfirmClicked()
 * @return true jika tombol konfirmasi telah diklik sejak Show() terakhir.
 */
bool Popup::IsConfirmClicked() const
{
    return confirmClicked;
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
        if (hasCancelButton) {
            confirmClicked = true;
        }
        Hide();
        return;
    }

    if (hasCancelButton && cancelButton.isClicked(mousePosition, mouseClicked)) {
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

    if (hasCancelButton) {
        int cancelWidth = MeasureText(cancelText, fontSize);
        int buttonsTotalWidth = buttonWidth + 20 + cancelWidth;

        width = std::max(textWidth, buttonsTotalWidth) + (paddingX * 2);
        width = std::min(width, maxWidth);

        height = fontSize + (fontSize * 2) + (paddingY * 3) + textPadding;

        position.x = (GameScreenWidth - width) / 2.0F;
        position.y = (GameScreenHeight - height) / 2.0F;

        backgroundRect = {position.x, position.y, static_cast<float>(width), static_cast<float>(height)};

        int startX = static_cast<int>(position.x + ((width - buttonsTotalWidth) / 2.0F));
        int buttonY = static_cast<int>(position.y + height - paddingY - fontSize);

        okButton = buttonTxt(buttonText, startX, buttonY, fontSize, WHITE, hoverAmount);
        cancelButton = buttonTxt(cancelText, startX + buttonWidth + 20, buttonY, fontSize, WHITE, hoverAmount);
    } else {
        width = std::max(textWidth, buttonWidth) + (paddingX * 2);
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
    int textY = static_cast<int>(position.y + (hasCancelButton ? 20 : 30));

    DrawText(message, textX, textY, 30, WHITE);

    okButton.Draw(mousePosition);
    if (hasCancelButton) {
        cancelButton.Draw(mousePosition);
    }
}
