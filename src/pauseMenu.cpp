/**
 * @file pauseMenu.cpp
 * @brief Implementasi dari Pause Menu System Module
 *
 * Implementasi dari class PauseMenu yang dideklarasikan di pauseMenu.h
 * Handle pause menu UI, tombol-tombol, dan popup notifikasi.
 */

#include "../include/pauseMenu.h"
#include "../include/popup.h"

/*==============================================================================
 * Static Variables (Popup Notifications)
 *==============================================================================*/

/** Popup notifikasi buat save game */
static Popup savePopup("Game Saved!", "OK", 0.7F);

/** Popup notifikasi buat load game */
static Popup loadPopup("Game Loaded!", "OK", 0.7F);

/** Popup notifikasi buat fitur "Coming Soon" (Options) */
static Popup pauseOptionsPopup("Coming Soon!", "OK", 0.7F);

/*==============================================================================
 * Constructor & Destructor
 *==============================================================================*/

/**
 * @brief Default constructor.
 * @note Inisialisasi semua tombol menu: Resume, Save, Load, Options, Return, Close
 */
PauseMenu::PauseMenu() : active(false), position({0, 0}), width(0), height(0)
{
    // Inisialisasi array teks tombol (6 buah)
    buttonTexts = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    buttonTexts[0] = "Resume";
    buttonTexts[1] = "Save Game";
    buttonTexts[2] = "Load Game";
    buttonTexts[3] = "Options";
    buttonTexts[4] = "Return to Main Menu";
    buttonTexts[5] = "Close Game";

    // Hitung dimensi menu berdasarkan teks
    CalculateDimensions();
}

/**
 * @brief Destructor.
 * @note Gak perlu cleanup khusus karena buttonTxt dan popup handle sendiri
 */
PauseMenu::~PauseMenu()
{
}

/*==============================================================================
 * Public Methods - Show/Hide/Active
 *==============================================================================*/

/**
 * @brief Show()
 * Tampilkan pause menu.
 */
void PauseMenu::Show()
{
    CalculateDimensions(); // Recalculate in case screen size changed
    active = true;
}

/**
 * @brief Hide()
 * Sembunyikan pause menu.
 */
void PauseMenu::Hide()
{
    active = false;
}

/**
 * @brief IsActive()
 * @return true jika pause menu sedang aktif.
 */
bool PauseMenu::IsActive() const
{
    return active;
}

/*==============================================================================
 * Private Methods - UI Calculation
 *==============================================================================*/

/**
 * @brief CalculateDimensions()
 * Hitung dimensi sidebar berdasarkan teks button.
 * @note Menu berbentuk sidebar di kiri layar dengan lebar 30% dari GameScreenWidth
 */
void PauseMenu::CalculateDimensions()
{
    const int maxWidth = static_cast<int>(GameScreenWidth * 0.30F); // sidebar lebar 30% layar
    const int fontSize = 30;
    const int paddingX = 20;
    const int paddingY = 20;
    const int buttonSpacing = 10;
    const int separatorSpacing = 30; // spasi ekstra buat separator antar grup

    // Step 1: Cari lebar tombol terpanjang buat acuan
    int maxButtonWidth = 0;
    for (std::uint8_t i = 0; i < 6; i++)
    {
        int btnWidth = MeasureText(buttonTexts[i], fontSize);
        if (btnWidth > maxButtonWidth)
        {
            maxButtonWidth = btnWidth;
        }
    }

    // Step 2: Set dimensi sidebar
    width = maxWidth;
    height = GameScreenHeight;

    position.x = 0; // sidebar nempel di kiri
    position.y = 0; // sidebar dari atas sampe bawah

    backgroundRect = {position.x, position.y, static_cast<float>(width), static_cast<float>(height)};

    // Step 3: Hitung posisi tiap tombol (centered horizontal di dalam sidebar)
    for (std::uint8_t i = 0; i < 6; i++)
    {
        int btnWidth = MeasureText(buttonTexts[i], fontSize);
        int btnX = position.x + (width - btnWidth) / 2; // center horizontal

        // Hitung spasi tambahan buat separator antar grup
        // Grup: Resume | Save+Load | Options+Return+Close
        int separatorCount = 0;
        if (i > 0)
            separatorCount++; // setelah Resume
        if (i > 2)
            separatorCount++; // setelah Save+Load
        if (i > 4)
            separatorCount++; // setelah Options+Return

        int btnY = position.y + paddingY + i * (fontSize + buttonSpacing) + separatorCount * (separatorSpacing - buttonSpacing);
        buttons[i] = buttonTxt(buttonTexts[i], btnX, btnY, fontSize, WHITE, 0.7F);
    }
}

/*==============================================================================
 * Private Methods - Button Handlers
 *==============================================================================*/

/**
 * @brief HandleButtonClick()
 * Handle klik tombol berdasarkan index.
 * @param buttonIndex Index tombol yang diklik (0-5)
 * @param state Game state saat ini (buat ganti screen atau keluar)
 */
void PauseMenu::HandleButtonClick(int buttonIndex, GameState *state)
{
    switch (buttonIndex)
    {
    case 0: // Resume - lanjut main
        Hide();
        break;
    case 1: // Save Game - tampilin popup notifikasi
        savePopup.Show();
        break;
    case 2: // Load Game - tampilin popup notifikasi
        loadPopup.Show();
        break;
    case 3: // Options - tampilin popup "Coming Soon"
        pauseOptionsPopup.Show();
        break;
    case 4: // Return to Main Menu - balik ke menu utama
        state->currentScreen = MAIN_MENU;
        Hide();
        break;
    case 5: // Close Game - keluar dari aplikasi
        CloseWindow();
        break;
    default:
        break;
    }
}

/*==============================================================================
 * Public Methods - Update & Draw
 *==============================================================================*/

/**
 * @brief Update()
 * Update state pause menu - handle klik tombol.
 * @param state Game state saat ini.
 * @param mousePosition Posisi mouse saat ini.
 * @param mouseClicked true jika tombol mouse diklik.
 * @note Popup bersifat modal - blocking interaksi menu selama aktif
 */
void PauseMenu::Update(GameState *state, Vector2 mousePosition, bool mouseClicked)
{
    if (!active)
    {
        return;
    }

    // Popup modal: kalo ada popup aktif, handle popup dulu, menu gak bisa diklik
    if (savePopup.IsActive())
    {
        savePopup.Update(mousePosition, mouseClicked);
        return;
    }

    if (loadPopup.IsActive())
    {
        loadPopup.Update(mousePosition, mouseClicked);
        return;
    }

    if (pauseOptionsPopup.IsActive()) {
        pauseOptionsPopup.Update(mousePosition, mouseClicked);
        return;
    }

    // Loop semua tombol, cek mana yang diklik
    for (std::uint8_t i = 0; i < 6; i++)
    {
        if (buttons[i].isClicked(mousePosition, mouseClicked))
        {
            HandleButtonClick(i, state);
        }
    }
}

/**
 * @brief Draw()
 * Render pause menu: background dan button.
 * @param mousePosition Posisi mouse saat ini (buat efek hover tombol)
 */
void PauseMenu::Draw(Vector2 mousePosition)
{
    if (!active)
    {
        return;
    }

    // Overlay gelap di seluruh layar (20% opacity)
    Rectangle fullScreen = {0, 0, static_cast<float>(GameScreenWidth), static_cast<float>(GameScreenHeight)};
    Color dimColor = {0, 0, 0, static_cast<unsigned char>(255 * 0.2F)};
    DrawRectangleRec(fullScreen, dimColor);

    // Sidebar background
    Color bgColor = DARKGRAY;
    DrawRectangleRec(backgroundRect, bgColor);
    DrawRectangleLines(
        static_cast<int>(backgroundRect.x),
        static_cast<int>(backgroundRect.y),
        static_cast<int>(backgroundRect.width),
        static_cast<int>(backgroundRect.height),
        WHITE);

    // Render semua tombol
    for (std::uint8_t i = 0; i < 6; i++)
    {
        buttons[i].Draw(mousePosition);
    }

    // Render popup kalo aktif (di atas menu)
    if (savePopup.IsActive())
    {
        savePopup.Draw(mousePosition);
    }

    if (loadPopup.IsActive())
    {
        loadPopup.Draw(mousePosition);
    }

    if (pauseOptionsPopup.IsActive()) {
        pauseOptionsPopup.Draw(mousePosition);
    }
}