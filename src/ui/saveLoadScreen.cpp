/**
 * @file saveLoadScreen.cpp
 * @brief Menu Simpan/Muat Game Module
 *
 * Implementasi kelas SaveLoadScreen untuk menangani
 * UI menu simpan dan muat game.
 */

#include "../../include/ui/saveLoadScreen.h"
#include "../lib/json/include/nlohmann/json.hpp"

/*==============================================================================
 * Constructor / Destructor
 *==============================================================================*/

/**
 * @brief Constructor
 *
 * Menginisialisasi semua member dan tombol navigasi.
 * Menggunakan buttonTxt (berbasis teks) untuk tombol BACK.
 */
SaveLoadScreen::SaveLoadScreen()
    : active(false)
    , texturesLoaded(false)
    , returnScreen(PLAY)
    , width(0)
    , height(0)
    , startX(0)
    , startY(0)
    , bgTexture({0})
    , slotOccupied{}
    , slotMapName{}
    , slotTimestamp{}
{
}

/**
 * @brief Destructor
 *
 * Membersihkan resource texture background jika sudah dimuat.
 */
SaveLoadScreen::~SaveLoadScreen()
{
    if (bgTexture.id != 0) {
        UnloadTexture(bgTexture);
    }
}

/*==============================================================================
 * Public Methods
 *==============================================================================*/

/**
 * @brief Menampilkan layar save/load
 *
 * Mengaktifkan flag active dan menghitung ulang dimensi UI.
 * Texture background akan dimuat saat Show() pertama kali dipanggil.
 */
void SaveLoadScreen::Show()
{
    active = true;
    if (!texturesLoaded) {
        // Texture akan dimuat di task mendatang jika diperlukan
        texturesLoaded = true;
    }
    CalculateDimensions();
    RefreshSlotMetadata();
}

/**
 * @brief Menyembunyikan layar save/load
 */
void SaveLoadScreen::Hide()
{
    active = false;
}

/**
 * @brief Memeriksa apakah layar save/load sedang aktif
 * @return true jika aktif, false jika tidak
 */
bool SaveLoadScreen::IsActive() const
{
    return active;
}

/**
 * @brief Mengatur layar kembali saat BACK diklik
 * @param screen Layar tujuan
 */
void SaveLoadScreen::SetReturnScreen(ScreenState screen)
{
    returnScreen = screen;
}

/*==============================================================================
 * Update & Draw
 *==============================================================================*/

/**
 * @brief Memperbarui handling input
 * @param state Pointer ke GameState
 * @param mousePosition Posisi mouse saat ini
 * @param mouseClicked Status klik mouse
 *
 * Memeriksa tombol BACK. Jika diklik, menyembunyikan layar
 * dan mengembalikan state ke returnScreen.
 */
void SaveLoadScreen::Update(GameState* state, Vector2 mousePosition, bool mouseClicked)
{
    if (!active) {
        return;
    }

    if (backButton.isClicked(mousePosition, mouseClicked)) {
        active = false;
        state->currentScreen = returnScreen;
        return;
    }
}

/**
 * @brief Me-render layar save/load
 * @param mousePosition Posisi mouse untuk efek hover
 *
 * Menggambar background menu, judul "SAVE / LOAD GAME",
 * grid slot manual dan autosave, serta tombol BACK.
 */
void SaveLoadScreen::Draw(Vector2 mousePosition)
{
    if (!active) {
        return;
    }

    DrawMenuBackground();

    if (bgTexture.id != 0) {
        DrawTexture(bgTexture, startX, startY, WHITE);
    } else {
        Color bgColor = {40, 40, 40, 230};
        DrawRectangleRec(backgroundRect, bgColor);
        DrawRectangleLinesEx(backgroundRect, 2, WHITE);
    }

    // Draw header
    const char* headerText = "SAVE / LOAD GAME";
    int headerFontSize = 24;
    int headerTextWidth = MeasureText(headerText, headerFontSize);
    int headerX = startX + (width - headerTextWidth) / 2;
    DrawText(headerText, headerX, startY + 18, headerFontSize, WHITE);

    // Draw slot grid
    DrawSlotGrid(mousePosition);

    backButton.Draw(mousePosition);
}

/*==============================================================================
 * Private Methods
 *==============================================================================*/

/**
 * @brief Menghitung dimensi dan posisi elemen UI
 *
 * Mengatur ukuran panel (600x400), memusatkannya di layar,
 * dan memposisikan tombol BACK di pojok kanan bawah panel.
 */
void SaveLoadScreen::CalculateDimensions()
{
    width = 850;
    height = 500;
    startX = (GameScreenWidth - width) / 2;
    startY = (GameScreenHeight - height) / 2;

    backgroundRect = {
        static_cast<float>(startX),
        static_cast<float>(startY),
        static_cast<float>(width),
        static_cast<float>(height)
    };

    backButton = buttonTxt(
        "BACK",
        startX + width - 100,
        startY + height - 50,
        24,
        WHITE,
        0.7F);
}

/**
 * @brief Gambar satu slot box
 * @param slotIndex Indeks slot (0-9)
 * @param posX Posisi X slot
 * @param posY Posisi Y slot
 * @param occupied Apakah slot terisi data
 * @param mapName Nama map yang ditampilkan
 * @param timestamp Timestamp save
 * @param mousePosition Posisi mouse untuk efek hover
 */
void SaveLoadScreen::DrawSlotBox(int slotIndex, int posX, int posY, bool occupied, const std::string& mapName, const std::string& timestamp, Vector2 mousePosition)
{
    Rectangle slotRect = {
        static_cast<float>(posX),
        static_cast<float>(posY),
        static_cast<float>(SLOT_WIDTH),
        static_cast<float>(SLOT_HEIGHT)
    };

    bool hovered = CheckCollisionPointRec(mousePosition, slotRect);

    Color bgColor;
    if (hovered) {
        bgColor = {70, 70, 100, 220};
    } else if (occupied) {
        bgColor = {50, 50, 70, 220};
    } else {
        bgColor = {30, 30, 40, 180};
    }

    DrawRectangleRec(slotRect, bgColor);

    Color borderColor = occupied ? (hovered ? WHITE : (Color){180, 180, 200, 255}) : GRAY;
    DrawRectangleLinesEx(slotRect, 1, borderColor);

    DrawText(TextFormat("Slot %d", slotIndex), posX + 5, posY + 5, 12, LIGHTGRAY);

    if (occupied) {
        DrawText(mapName.c_str(), posX + 5, posY + 22, 14, WHITE);
        DrawText(timestamp.c_str(), posX + 5, posY + 44, 10, (Color){180, 180, 180, 255});
    } else {
        const char* emptyText = "Empty";
        int emptyTextWidth = MeasureText(emptyText, 16);
        int emptyX = posX + (SLOT_WIDTH - emptyTextWidth) / 2;
        int emptyY = posY + (SLOT_HEIGHT - 16) / 2;
        DrawText(emptyText, emptyX, emptyY, 16, GRAY);
    }
}

/**
 * @brief Gambar grid slot manual dan autosave
 * @param mousePosition Posisi mouse untuk efek hover
 *
 * Layout: 3+2 untuk manual, 3+2 untuk autosave.
 * Manual: slot 0-4, Autosave: slot 5-9.
 */
void SaveLoadScreen::DrawSlotGrid(Vector2 mousePosition)
{
    DrawText("MANUAL SAVE", startX + 10, startY + 50, 18, WHITE);

    int manualRow1Y = startY + 75;
    int rowWidth3 = 3 * SLOT_WIDTH + 2 * SLOT_GAP;
    int row1X = startX + (width - rowWidth3) / 2;

    for (int i = 0; i < 3; i++) {
        int slotX = row1X + i * (SLOT_WIDTH + SLOT_GAP);
        DrawSlotBox(i, slotX, manualRow1Y, slotOccupied[i], slotMapName[i], slotTimestamp[i], mousePosition);
    }

    int manualRow2Y = manualRow1Y + SLOT_HEIGHT + SLOT_GAP;
    int rowWidth2 = 2 * SLOT_WIDTH + 1 * SLOT_GAP;
    int row2X = startX + (width - rowWidth2) / 2;

    for (int i = 3; i < 5; i++) {
        int slotX = row2X + (i - 3) * (SLOT_WIDTH + SLOT_GAP);
        DrawSlotBox(i, slotX, manualRow2Y, slotOccupied[i], slotMapName[i], slotTimestamp[i], mousePosition);
    }

    int autoLabelY = manualRow2Y + SLOT_HEIGHT + 15;
    DrawText("AUTO SAVE", startX + 10, autoLabelY, 18, WHITE);

    int autoRow1Y = autoLabelY + 25;

    for (int i = 5; i < 8; i++) {
        int slotX = row1X + (i - 5) * (SLOT_WIDTH + SLOT_GAP);
        DrawSlotBox(i, slotX, autoRow1Y, slotOccupied[i], slotMapName[i], slotTimestamp[i], mousePosition);
    }

    int autoRow2Y = autoRow1Y + SLOT_HEIGHT + SLOT_GAP;

    for (int i = 8; i < 10; i++) {
        int slotX = row2X + (i - 8) * (SLOT_WIDTH + SLOT_GAP);
        DrawSlotBox(i, slotX, autoRow2Y, slotOccupied[i], slotMapName[i], slotTimestamp[i], mousePosition);
    }
}

/**
 * @brief Muat metadata semua slot dari disk
 *
 * Untuk setiap slot N (0-9), periksa saves/slot_N/manual/manual.json.
 * Jika ada, baca mapDisplayName dan timestamp.
 * Jika tidak, tandai sebagai kosong.
 */
void SaveLoadScreen::RefreshSlotMetadata()
{
    for (int i = 0; i < MANUAL_SLOT_COUNT + AUTOSAVE_SLOT_COUNT; i++)
    {
        std::string path = "saves/slot_" + std::to_string(i) + "/manual/manual.json";

        if (std::filesystem::exists(path))
        {
            slotOccupied[i] = true;
            try {
                std::ifstream file(path);
                nlohmann::json root;
                file >> root;

                slotMapName[i] = root.value("mapDisplayName", "Unknown");

                if (root.contains("timestamp")) {
                    slotTimestamp[i] = root["timestamp"].get<std::string>();
                } else {
                    auto ftime = std::filesystem::last_write_time(path);
                    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now());
                    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

                    char timeStr[64];
                    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M", std::localtime(&cftime));
                    slotTimestamp[i] = timeStr;
                }

                file.close();
            } catch (...) {
                slotMapName[i] = "Error";
                slotTimestamp[i] = "";
            }
        }
        else
        {
            slotOccupied[i] = false;
            slotMapName[i] = "";
            slotTimestamp[i] = "";
        }
    }
}
