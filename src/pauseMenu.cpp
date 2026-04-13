#include "../include/pauseMenu.h"
#include "../include/popup.h"

static Popup savePopup;
static Popup loadPopup;

/**
 * @brief Default constructor.
 */
PauseMenu::PauseMenu() : active(false), position({0, 0}), width(0), height(0)
{
    buttonTexts = {"Resume", "Save Game", "Load Game", "Return to Main Menu", "Exit Game"};
    CalculateDimensions();
}

/**
 * @brief Destructor.
 */
PauseMenu::~PauseMenu()
{
}

/**
 * @brief Show()
 * Tampilkan pause menu.
 */
void PauseMenu::Show()
{
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

/**
 * @brief CalculateDimensions()
 * Hitung dimensi sidebar berdasarkan teks button.
 */
void PauseMenu::CalculateDimensions()
{
    const int maxWidth = static_cast<int>(GameScreenWidth * 0.25F);
    const int fontSize = 30;
    const int paddingX = 20;
    const int paddingY = 20;
    const int buttonSpacing = 10;

    int maxButtonWidth = 0;
    for (std::uint8_t i = 0; i < 5; i++) {
        int btnWidth = MeasureText(buttonTexts[i], fontSize);
        if (btnWidth > maxButtonWidth) {
            maxButtonWidth = btnWidth;
        }
    }

    width = maxButtonWidth + (paddingX * 2);
    if (width > maxWidth) {
        width = maxWidth;
    }

    height = (fontSize + buttonSpacing) * 5 + (paddingY * 2);

    position.x = 0;
    position.y = (GameScreenHeight - height) / 2.0F;

    backgroundRect = {position.x, position.y, static_cast<float>(width), static_cast<float>(height)};

    for (std::uint8_t i = 0; i < 5; i++) {
        int btnWidth = MeasureText(buttonTexts[i], fontSize);
        int btnX = position.x + (width - btnWidth) / 2;
        int btnY = position.y + paddingY + i * (fontSize + buttonSpacing);
        buttons[i] = buttonTxt(buttonTexts[i], btnX, btnY, fontSize, WHITE, 1.0F);
    }
}

/**
 * @brief HandleButtonClick()
 * Handle klik tombol berdasarkan index.
 * @param buttonIndex Index tombol yang diklik.
 * @param state Game state saat ini.
 */
void PauseMenu::HandleButtonClick(int buttonIndex, GameState* state)
{
    switch (buttonIndex) {
        case 0:  // Resume
            Hide();
            break;
        case 1:  // Save Game
            savePopup.Show();
            break;
        case 2:  // Load Game
            loadPopup.Show();
            break;
        case 3:  // Return to Main Menu
            state->currentScreen = MAIN_MENU;
            Hide();
            break;
        case 4:  // Exit Game
            CloseWindow();
            break;
        default:
            break;
    }
}

/**
 * @brief Update()
 * Update state pause menu - handle klik tombol.
 * @param state Game state saat ini.
 * @param mousePosition Posisi mouse saat ini.
 * @param mouseClicked true jika tombol mouse diklik.
 */
void PauseMenu::Update(GameState* state, Vector2 mousePosition, bool mouseClicked)
{
    if (!active) {
        return;
    }

    if (savePopup.IsActive()) {
        savePopup.Update(mousePosition, mouseClicked);
        return;
    }

    if (loadPopup.IsActive()) {
        loadPopup.Update(mousePosition, mouseClicked);
        return;
    }

    for (std::uint8_t i = 0; i < 5; i++) {
        if (buttons[i].isClicked(mousePosition, mouseClicked)) {
            HandleButtonClick(i, state);
        }
    }
}

/**
 * @brief Draw()
 * Render pause menu: background dan button.
 * @param mousePosition Posisi mouse saat ini.
 */
void PauseMenu::Draw(Vector2 mousePosition)
{
    if (!active) {
        return;
    }

    Rectangle fullScreen = {0, 0, static_cast<float>(GameScreenWidth), static_cast<float>(GameScreenHeight)};
    Color dimColor = {0, 0, 0, static_cast<unsigned char>(255 * 0.2F)};
    DrawRectangleRec(fullScreen, dimColor);

    Color bgColor = DARKGRAY;
    DrawRectangleRec(backgroundRect, bgColor);
    DrawRectangleLines(
        static_cast<int>(backgroundRect.x),
        static_cast<int>(backgroundRect.y),
        static_cast<int>(backgroundRect.width),
        static_cast<int>(backgroundRect.height),
        WHITE);

    for (std::uint8_t i = 0; i < 5; i++) {
        buttons[i].Draw(mousePosition);
    }

    if (savePopup.IsActive()) {
        savePopup.Draw(mousePosition);
    }

    if (loadPopup.IsActive()) {
        loadPopup.Draw(mousePosition);
    }
}