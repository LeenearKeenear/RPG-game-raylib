/**
 * @file popup.cpp
 * @brief Implementasi dari Popup Dialog System Module
 *
 * Implementasi dari class Popup yang dideklarasikan di popup.h
 * Handle popup dialog dengan pesan dan tombol OK.
 */

#include <algorithm>

#include "../include/popup.h"
#include "../include/screen.h"

/*==============================================================================
 * Constructors & Destructor
 *==============================================================================*/

/**
 * @brief Default constructor.
 * @note Bikin popup kosong, perlu di-set message dan buttonText nanti
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
Popup::Popup(const char *message, const char *buttonText, float hoverAmount)
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

/*==============================================================================
 * Public Methods - Show/Hide/Active
 *==============================================================================*/

/**
 * @brief Show()
 * Tampilkan popup ke layar.
 */
void Popup::Show()
{
    CalculateDimensions(); // Recalculate in case screen size changed
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

/*==============================================================================
 * Public Methods - Update & Draw
 *==============================================================================*/

/**
 * @brief Update()
 * Update state popup - handle klik tombol OK.
 * @param mousePosition Posisi mouse saat ini.
 * @param mouseClicked true jika tombol mouse diklik.
 */
void Popup::Update(Vector2 mousePosition, bool mouseClicked)
{
    if (!active)
    {
        return;
    }

    // Kalo tombol OK diklik, tutup popup
    if (okButton.isClicked(mousePosition, mouseClicked))
    {
        Hide();
    }
}

/*==============================================================================
 * Private Methods - UI Calculation
 *==============================================================================*/

/**
 * @brief CalculateDimensions()
 * Hitung dimensi popup berdasarkan teks message dan button.
 * @note Popup di-center di layar, lebar maksimal 60% dari GameScreenWidth
 */
void Popup::CalculateDimensions()
{
    const int paddingX = 40;                                       // padding horizontal dalam popup
    const int paddingY = 30;                                       // padding vertikal dalam popup
    const int textPadding = 20;                                    // spasi ekstra buat teks
    const int maxWidth = static_cast<int>(GameScreenWidth * 0.6F); // lebar maks 60% layar
    const int fontSize = 30;

    // Step 1: Ukur lebar teks message dan button
    int textWidth = MeasureText(message, fontSize);
    int buttonWidth = MeasureText(buttonText, fontSize);

    // Step 2: Hitung lebar popup (ambil yang terbesar antara text dan button)
    width = textWidth + (paddingX * 2);
    width = std::max(buttonWidth + (paddingX * 2), width);
    width = std::min(width, maxWidth); // Clamp ke maxWidth

    // Step 3: Hitung tinggi popup
    // Rumus: message + spasi + button + padding
    height = fontSize + (fontSize * 2) + (paddingY * 3) + textPadding;

    // Step 4: Posisi popup di tengah layar
    position.x = (GameScreenWidth - width) / 2.0F;
    position.y = (GameScreenHeight - height) / 2.0F;

    backgroundRect = {position.x, position.y, static_cast<float>(width), static_cast<float>(height)};

    // Step 5: Posisi tombol OK (centered horizontal di bagian bawah popup)
    int buttonX = static_cast<int>(position.x + ((width - buttonWidth) / 2.0F));
    int buttonY = static_cast<int>(position.y + height - paddingY - fontSize);

    // Debug logs (kalo perlu)
    TraceLog(LOG_DEBUG, "Popup Debug: width=%d, height=%d", width, height);
    TraceLog(LOG_DEBUG, "Popup Debug: position=(%.1f, %.1f)", position.x, position.y);
    TraceLog(LOG_DEBUG, "Popup Debug: buttonX=%d, buttonY=%d, buttonWidth=%d", buttonX, buttonY, buttonWidth);

    // Step 6: Inisialisasi tombol OK
    okButton = buttonTxt(buttonText, buttonX, buttonY, fontSize, WHITE, hoverAmount);
}

/**
 * @brief Draw()
 * Render popup: overlay, background, message, dan button.
 * @param mousePosition Posisi mouse saat ini.
 */
void Popup::Draw(Vector2 mousePosition)
{
    if (!active)
    {
        return;
    }

    // ===== Overlay gelap di seluruh layar (50% opacity) =====
    Color overlayColor = {
        0,
        0,
        0,
        static_cast<unsigned char>(255 * 0.5F)};

    Rectangle fullScreen = {0, 0, static_cast<float>(GameScreenWidth), static_cast<float>(GameScreenHeight)};
    DrawRectangleRec(fullScreen, overlayColor);

    // ===== Background popup =====
    Color bgColor = {20, 20, 20, 255}; // abu-abu gelap
    DrawRectangleRec(backgroundRect, bgColor);
    DrawRectangleLines(
        static_cast<int>(backgroundRect.x),
        static_cast<int>(backgroundRect.y),
        static_cast<int>(backgroundRect.width),
        static_cast<int>(backgroundRect.height),
        WHITE);

    // ===== Render pesan teks =====
    int textWidth = MeasureText(message, 30);
    int textX = static_cast<int>(position.x + ((width - textWidth) / 2.0F)); // center horizontal
    int textY = static_cast<int>(position.y + 30);                           // offset dari atas

    DrawText(message, textX, textY, 30, WHITE);

    // ===== Render tombol OK =====
    okButton.Draw(mousePosition);
}