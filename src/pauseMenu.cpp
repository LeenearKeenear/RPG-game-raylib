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

/*==============================================================================
 * Constructor & Destructor
 *==============================================================================*/

/**
 * @brief Default constructor.
 * @note Inisialisasi semua tombol menu: Resume, Save, Load, Options, Return, Close
 */
PauseMenu::PauseMenu() : active(false), optionsActive(false), position({0, 0}), width(0), height(0), selectedResolution(0)
{
    // Inisialisasi array teks tombol (6 buah)
    buttonTexts = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    buttonTexts[0] = "Resume";
    buttonTexts[1] = "Save Game";
    buttonTexts[2] = "Load Game";
    buttonTexts[3] = "Options";
    buttonTexts[4] = "Return to Main Menu";
    buttonTexts[5] = "Close Game";

    // Initialize resolution options
    resolutionOptions = GetAvailableResolutions();

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
    optionsActive = false;
}

/**
 * @brief Hide()
 * Sembunyikan pause menu.
 */
void PauseMenu::Hide()
{
    active = false;
    optionsActive = false;
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
 * Video Settings Methods
 *==============================================================================*/

/**
 * @brief GetAvailableResolutions()
 * Ambil resolusi yang tersedia berdasarkan monitor.
 * @return Vector of ResOption (width, height, label)
 */
std::vector<ResOption> PauseMenu::GetAvailableResolutions()
{
    std::vector<ResOption> options;

    // Get monitor resolution
    Rectangle monitor = GetMonitorResolution();
    int maxWidth = static_cast<int>(monitor.width);
    int maxHeight = static_cast<int>(monitor.height);

    // Add available resolutions (always include 720p)
    if (1280 <= maxWidth && 720 <= maxHeight) {
        options.push_back({1280, 720, "720p"});
    }
    if (1920 <= maxWidth && 1080 <= maxHeight) {
        options.push_back({1920, 1080, "1080p"});
    }
    if (2560 <= maxWidth && 1440 <= maxHeight) {
        options.push_back({2560, 1440, "1440p"});
    }
    if (3840 <= maxWidth && 2160 <= maxHeight) {
        options.push_back({3840, 2160, "4K"});
    }

    // Default to 720p if no options
    if (options.empty()) {
        options.push_back({1280, 720, "720p"});
    }

    return options;
}

/**
 * @brief ToggleFullscreen()
 * Toggle antara fullscreen dan windowed.
 */
void PauseMenu::ToggleFullscreen(GameState* state)
{
    (void)state; // unused for now
    ToggleFullscreenMode();
}

/**
 * @brief CycleResolution()
 * Cycle melalui resolusi yang tersedia.
 * @param forward true = next, false = previous
 */
void PauseMenu::CycleResolution(GameState* state)
{
    if (resolutionOptions.empty()) return;

    // Get current window size
    int currentWidth = GetScreenWidth();
    int currentHeight = GetScreenHeight();

    // Find current index
    int currentIndex = 0;
    for (size_t i = 0; i < resolutionOptions.size(); i++) {
        if (resolutionOptions[i].width == currentWidth && resolutionOptions[i].height == currentHeight) {
            currentIndex = static_cast<int>(i);
            break;
        }
    }

    // Auto-detect current resolution if not in list (windowed mode vs fullscreen)
    if (currentIndex == 0 && resolutionOptions.size() > 1) {
        // Check if current matches next option
        for (size_t i = 1; i < resolutionOptions.size(); i++) {
            if (resolutionOptions[i].width == currentWidth && resolutionOptions[i].height == currentHeight) {
                currentIndex = static_cast<int>(i);
                break;
            }
        }
    }

    // Cycle to next (wrap around)
    selectedResolution = (currentIndex + 1) % static_cast<int>(resolutionOptions.size());

    // Apply new resolution (if in windowed mode)
    if (!IsFullscreen()) {
        SetResolution(resolutionOptions[selectedResolution].width, resolutionOptions[selectedResolution].height);
    }
}

/**
 * @brief ToggleFPS()
 * Toggle menampilkan FPS counter.
 */
void PauseMenu::ToggleFPS(GameState* state)
{
    state->showFPS = !state->showFPS;
}

/*==============================================================================
 * Private Methods - UI Calculation
 *==============================================================================*/

/**
 * @brief CalculateDimensions()
 * Hitung dimensi menu berdasarkan teks button.
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

    // Step 4: Calculate options screen buttons
    // Resolution option button
    if (!resolutionOptions.empty()) {
        const char* resLabel = resolutionOptions[selectedResolution].label;
        int resBtnWidth = MeasureText(resLabel, fontSize);
        buttons[3] = buttonTxt("Resolution", position.x + (width - MeasureText("Resolution", fontSize)) / 2, 200, fontSize, WHITE, 0.7F);
        buttonTxt resValueBtn(resLabel, position.x + (width - resBtnWidth) / 2, 240, fontSize, YELLOW, 0.7F);
        
        // Placeholder - we're cycling instead of clicking the value
    } else {
        buttons[3] = buttonTxt("Resolution", 50, 200, fontSize, WHITE, 0.7F);
    }

    // Back button for options screen
    const char* backText = "BACK";
    int backWidth = MeasureText(backText, fontSize);
    backButton = buttonTxt(backText, position.x + (width - backWidth) / 2, 500, fontSize, WHITE, 0.7F);
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
    if (optionsActive)
    {
        // Handle options screen buttons
        if (buttonIndex == 99) // Back button
        {
            optionsActive = false;
        }
        return;
    }

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
        case 3:  // Options
            optionsActive = true;
            CalculateDimensions();
            break;
        case 4:  // Return to Main Menu
            state->currentScreen = MAIN_MENU;
            Hide();
            break;
        case 5:  // Close Game
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

    // Handle options screen separately
    if (optionsActive)
    {
        // Back button click
        if (backButton.isClicked(mousePosition, mouseClicked))
        {
            optionsActive = false;
            return;
        }

        // Click on Resolution label - cycle resolution
        if (buttons[3].isClicked(mousePosition, mouseClicked))
        {
            CycleResolution(state);
            CalculateDimensions();
            return;
        }

        // Click on Fullscreen area (upper part of options)
        Rectangle fullscreenArea = {0, 100, static_cast<float>(width), 80};
        if (CheckCollisionPointRec(mousePosition, fullscreenArea) && mouseClicked)
        {
            ToggleFullscreen(state);
            return;
        }

        // Click on FPS area
        Rectangle fpsArea = {0, 280, static_cast<float>(width), 80};
        if (CheckCollisionPointRec(mousePosition, fpsArea) && mouseClicked)
        {
            ToggleFPS(state);
            return;
        }

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

    if (optionsActive)
    {
        // Draw options screen
        DrawOptionsScreen(mousePosition);
    }
    else
    {
        // Render semua tombol
        for (std::uint8_t i = 0; i < 6; i++)
        {
            buttons[i].Draw(mousePosition);
        }
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
}

/**
 * @brief DrawOptionsScreen()
 * Render options screen: fullscreen, resolution, fps toggles.
 */
void PauseMenu::DrawOptionsScreen(Vector2 mousePosition)
{
    const int fontSize = 30;

    // Title
    DrawText("VIDEO SETTINGS", position.x + (width - MeasureText("VIDEO SETTINGS", fontSize)) / 2, 30, fontSize, WHITE);

    // Fullscreen toggle area (clickable)
    Rectangle fullscreenArea = {0, 100, static_cast<float>(width), 80};
    Color fsColor = {60, 60, 60, 255};
    DrawRectangleRec(fullscreenArea, fsColor);
    DrawText("Fullscreen", position.x + 20, 120, fontSize, WHITE);
    const char* fsStatus = IsFullscreen() ? "ON" : "OFF";
    Color fsStatusColor = IsFullscreen() ? GREEN : RED;
    DrawText(fsStatus, position.x + width - 80, 120, fontSize, fsStatusColor);

    // Resolution (clickable to cycle)
    Color resBgColor = {60, 60, 60, 255};
    Rectangle resArea = {0, 180, static_cast<float>(width), 80};
    DrawRectangleRec(resArea, resBgColor);
    DrawText("Resolution", position.x + 20, 200, fontSize, WHITE);
    if (!resolutionOptions.empty()) {
        const char* resLabel = resolutionOptions[selectedResolution].label;
        DrawText(resLabel, position.x + width - 80, 200, fontSize, YELLOW);
    }

    // Show FPS toggle area (clickable)
    Rectangle fpsArea = {0, 260, static_cast<float>(width), 80};
    Color fpsBgColor = {60, 60, 60, 255};
    DrawRectangleRec(fpsArea, fpsBgColor);
    DrawText("Show FPS", position.x + 20, 280, fontSize, WHITE);
    const char* fpsStatus = "OFF"; // We'll show current state via GameState
    DrawText(fpsStatus, position.x + width - 80, 280, fontSize, GRAY);

    // Back button
    backButton.Draw(mousePosition);
}