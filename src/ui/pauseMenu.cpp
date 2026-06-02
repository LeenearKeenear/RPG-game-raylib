/**
 * @file pauseMenu.cpp
 * @brief Implementasi dari Pause Menu System dan Options Screen
 *
 * Handle pause menu UI dan standalone options screen dengan tabs.
 */

#include <algorithm>

#include "pauseMenu.h"
#include "popup.h"
#include "videoTab.h"
#include "audioTab.h"
#include "keybindsTab.h"
#include "game_state_saver.h"
#include "player.h"
#include "worldgenio.h"
#include "seedmanager.h"

/*==============================================================================
 * Static Variables (Popup Notifications)
 *==============================================================================*/

/**
 * @brief Popup notifikasi untuk save game
 */
static Popup savePopup("Game Saved!", "OK", 0.7F);

/**
 * @brief Popup notifikasi untuk load game
 */
static Popup loadPopup("Game Loaded!", "OK", 0.7F);

/*==============================================================================
 * OptionsScreen Implementation
 *==============================================================================*/

/**
 * @brief Constructor
 * 
 * Menginisialisasi semua tombol tab, tombol back, dan dimensi awal.
 */
OptionsScreen::OptionsScreen() : active(false), texturesLoaded(false), returnScreen(PLAY), selectedTab(0), showFPS(false), width(0), height(0), startX(0), startY(0), bgTexture({0})
{
}

/**
 * @brief Destructor
 */
OptionsScreen::~OptionsScreen()
{
    if (bgTexture.id != 0) {
        UnloadTexture(bgTexture);
    }
}

/**
 * @brief Menampilkan layar options
 */
void OptionsScreen::Show()
{
    active = true;
    if (!texturesLoaded) {
        Image img = LoadImage("assets/textures/settingsButt/settingsBG.png");
        if (img.data != nullptr) {
            bgTexture = LoadTextureFromImage(img);
            UnloadImage(img);
        }
        texturesLoaded = true;
    }
    CalculateDimensions();
}

/**
 * @brief Menyembunyikan layar options
 */
void OptionsScreen::Hide()
{
    active = false;
}

/**
 * @brief Memeriksa apakah layar options sedang aktif
 * @return true jika aktif, false jika tidak
 */
bool OptionsScreen::IsActive() const
{
    return active;
}

/**
 * @brief Mengatur layar kembali saat BACK diklik
 * @param screen Layar tujuan
 */
void OptionsScreen::SetReturnScreen(ScreenState screen)
{
    returnScreen = screen;
}

/**
 * @brief Mendapatkan daftar resolusi yang tersedia berdasarkan monitor
 * @return Vektor berisi ResOption (width, height, label)
 */
std::vector<ResOption> GetAvailableResolutions()
{
    std::vector<ResOption> options;

    Rectangle monitor = GetMonitorResolution();
    int maxWidth = static_cast<int>(monitor.width);
    int maxHeight = static_cast<int>(monitor.height);

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

    if (options.empty()) {
        options.push_back({1280, 720, "720p"});
    }

    return options;
}

/**
 * @brief Menghitung dimensi dan membuat elemen UI
 * 
 * Menggunakan Approach B: selalu mulai dari opsi pertama (720p)
 * tanpa melakukan auto-detect resolusi saat ini.
 */
void OptionsScreen::CalculateDimensions()
{
    const int tabHeight = 56;

    width = bgTexture.width > 0 ? bgTexture.width : 800;
    height = bgTexture.height > 0 ? bgTexture.height : 600;
    startX = (GameScreenWidth - width) / 2;
    startY = (GameScreenHeight - height) / 2;

    backgroundRect = {static_cast<float>(startX), static_cast<float>(startY), static_cast<float>(width), static_cast<float>(height)};

    int tabY = startY + 15 + 120;

    const char* tabFiles[3] = {
        "assets/textures/settingsButt/settingsVideo.png",
        "assets/textures/settingsButt/settingsAudio.png",
        "assets/textures/settingsButt/settingsKeybinds.png"
    };
    for (int i = 0; i < 3; i++) {
        float cx = static_cast<float>(startX + width / 2 + (i - 1) * 302);
        float cy = static_cast<float>(tabY + tabHeight / 2);
        tabButtons[i] = buttonImage(tabFiles[i], Vector2{cx, cy}, 1.0F, 0.7F);
    }

    backButton = buttonImage(
        "assets/textures/settingsButt/settingsBack.png",
        Vector2{static_cast<float>(startX + width - 133),
                static_cast<float>(startY + height - 53)},
        1.0F, 0.7F);

    if (resolutionOptions.empty()) {
        resolutionOptions = GetAvailableResolutions();
    }

    const int labelFontSize = 24;
    int valueX = startX + 339;
    int contentStartY = startY + 221;

    bool isFullscreen = IsWindowFullscreen();
    fullscreenButton = buttonTxt(
        isFullscreen ? "ON" : "OFF",
        valueX,
        contentStartY + 15,
        labelFontSize,
        isFullscreen ? GREEN : RED,
        0.7F);

    fpsButton = buttonTxt(
        showFPS ? "ON" : "OFF",
        valueX,
        contentStartY + 75,
        labelFontSize,
        showFPS ? GREEN : GRAY,
        0.7F);
}

/**
 * @brief Memperbarui handling input
 * @param state Pointer ke GameState
 * @param mousePosition Posisi mouse saat ini
 * @param mouseClicked Status klik mouse
 */
void OptionsScreen::Update(GameState* state, Vector2 mousePosition, bool mouseClicked)
{
    if (!active) {
        return;
}

    if (backButton.isClicked(mousePosition, mouseClicked)) {
        active = false;
        state->currentScreen = returnScreen;
        return;
    }

    for (int i = 0; i < 3; i++) {
        if (tabButtons[i].isClicked(mousePosition, mouseClicked)) {
            selectedTab = i;
            CalculateDimensions();
            return;
        }
    }

    if (selectedTab == 0) {
        if (UpdateVideoTab(fullscreenButton, fpsButton, state, mousePosition, mouseClicked)) {
            showFPS = state->showFPS;
            CalculateDimensions();
        }
    }
}

/**
 * @brief Me-render layar options
 * @param mousePosition Posisi mouse untuk efek hover
 */
void OptionsScreen::Draw(Vector2 mousePosition)
{
    if (!active) {
        return;
    }

    if (bgTexture.id != 0) {
        DrawTexture(bgTexture, startX, startY, WHITE);
    } else {
        Color bgColor = {40, 40, 40, 230};
        DrawRectangleRec(backgroundRect, bgColor);
        DrawRectangleLinesEx(backgroundRect, 2, WHITE);
    }

    for (int i = 0; i < 3; i++) {
        tabButtons[i].Draw(mousePosition);
    }

    backButton.Draw(mousePosition);

    int contentOX = 89, contentOY = 121;
    Rectangle contentRect = {
        static_cast<float>(startX + contentOX + 20),
        static_cast<float>(startY + 200),
        static_cast<float>(width - 2 * (contentOX + 20)),
        static_cast<float>(height - 323)
    };
    DrawRectangleRec(contentRect, {0, 0, 0, 51});

    switch (selectedTab) {
        case 0: DrawVideoTab(fullscreenButton, fpsButton, mousePosition, startX + contentOX, startY + contentOY); break;
        case 1: DrawAudioTab(mousePosition, startX + contentOX, startY + contentOY); break;
        case 2: DrawKeybindsTab(mousePosition, startX + contentOX, startY + contentOY); break;
    }
}

/*==============================================================================
 * PauseMenu Implementation
 *==============================================================================*/

static const char* BUTTON_PATHS[6] = {
    "assets/textures/pauseButt/pause-resume.png",
    "assets/textures/pauseButt/pause-save.png",
    "assets/textures/pauseButt/pause-load.png",
    "assets/textures/pauseButt/pause-restart.png",
    "assets/textures/pauseButt/pause-tomain.png",
    "assets/textures/pauseButt/pause-exit.png"
};

/**
 * @brief Constructor
 */
PauseMenu::PauseMenu()
    : active(false), texturesLoaded(false), bgTexture({0}), position({0, 0}), width(0), height(0)
{
}

/**
 * @brief Destructor
 */
PauseMenu::~PauseMenu()
{
    if (bgTexture.id != 0)
        UnloadTexture(bgTexture);
}

/**
 * @brief Memuat texture dari disk (lazy, sekali saja)
 */
void PauseMenu::LoadTextures()
{
    if (texturesLoaded)
        return;
    texturesLoaded = true;

    Image img = LoadImage("assets/textures/pauseButt/pause-bg.png");
    bgTexture = LoadTextureFromImage(img);
    UnloadImage(img);

    width = bgTexture.width;
    height = bgTexture.height;
    position.x = (GameScreenWidth - width) / 2.0F;
    position.y = (GameScreenHeight - height) / 2.0F;
    backgroundRect = {position.x, position.y, static_cast<float>(width), static_cast<float>(height)};

    float centerX = position.x + width / 2.0F;
    float gap = 17.0F;
    float btnHeight = 56.0F;
    float totalBtnHeight = 6.0F * btnHeight + 5.0F * gap;
    float startY = position.y + (height - totalBtnHeight) / 2.0F + 30.0F;

    for (int i = 0; i < 6; i++)
    {
        float btnY = startY + i * (btnHeight + gap) + btnHeight / 2.0F;
        buttons[i] = buttonImage(BUTTON_PATHS[i], Vector2{centerX, btnY}, 1.0F, 0.6F);
    }
}

/**
 * @brief Menampilkan pause menu
 */
void PauseMenu::Show()
{
    LoadTextures();
    active = true;
}

/**
 * @brief Menyembunyikan pause menu
 */
void PauseMenu::Hide()
{
    active = false;
}

/**
 * @brief Memeriksa apakah pause menu sedang aktif
 * @return true jika aktif, false jika tidak
 */
bool PauseMenu::IsActive() const
{
    return active;
}

/**
 * @brief Menghitung ulang dimensi (dummy — virtual screen fixed, tidak perlu)
 */
void PauseMenu::CalculateDimensions()
{
    // Virtual screen 1280x720 is fixed; textures loaded once in LoadTextures()
}

/**
 * @brief Handle klik pada tombol berdasarkan index
 * @param buttonIndex Index tombol yang diklik (0-5)
 * @param state Pointer ke GameState
 */
void PauseMenu::HandleButtonClick(int buttonIndex, GameState* state)
{
    switch (buttonIndex) {
        case 0: // Resume
            Hide();
            break;
        case 1:
            // Save Game — simpan runtime + player state ke disk
            WorldgenIO::SaveRuntimeState(g_SeedManager.GetCurrentStage());
            SaveGameState(state);
            savePopup.Show();
            break;
        case 2: // Load
            loadPopup.Show();
            break;
        case 3: // Restart (New Game)
            ClearSavedState();
            PlayerInstance.ResetForNewGame();
            state->enteredLoading = false;
            state->loadingStage = 0;
            state->loadingProgress = 0.0F;
            state->loadingComplete = false;
            state->currentScreen = LOADING;
            Hide();
            break;
        case 4:
            // Return to Main Menu — save dulu baru pindah
            WorldgenIO::SaveRuntimeState(g_SeedManager.GetCurrentStage());
            SaveGameState(state);
            state->enteredLoading = false;
            state->loadingStage = 0;
            state->loadingProgress = 0.0F;
            state->loadingComplete = false;
            state->currentScreen = MAIN_MENU;
            Hide();
            break;
        case 5:
            // Close Game — save dulu baru tutup
            WorldgenIO::SaveRuntimeState(g_SeedManager.GetCurrentStage());
            SaveGameState(state);
            CloseWindow();
            break;
        default:
            break;
    }
}

/**
 * @brief Memperbarui logic pause menu
 * @param state Pointer ke GameState
 * @param mousePosition Posisi mouse saat ini
 * @param mouseClicked Status klik mouse
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

    for (std::uint8_t i = 0; i < 6; i++) {
        if (buttons[i].isClicked(mousePosition, mouseClicked)) {
            HandleButtonClick(i, state);
        }
    }
}

/**
 * @brief Me-render pause menu ke layar
 * @param mousePosition Posisi mouse untuk efek hover
 */
void PauseMenu::Draw(Vector2 mousePosition)
{
    if (!active) {
        return;
    }

    Rectangle fullScreen = {0, 0, static_cast<float>(GameScreenWidth), static_cast<float>(GameScreenHeight)};
    Color dimColor = {0, 0, 0, static_cast<unsigned char>(255 * 0.2F)};
    DrawRectangleRec(fullScreen, dimColor);

    DrawTextureV(bgTexture, position, WHITE);

    for (std::uint8_t i = 0; i < 6; i++) {
        buttons[i].Draw(mousePosition);
    }

    if (savePopup.IsActive()) {
        savePopup.Draw(mousePosition);
    }
    if (loadPopup.IsActive()) {
        loadPopup.Draw(mousePosition);
    }
}
