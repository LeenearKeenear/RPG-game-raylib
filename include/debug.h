#pragma once

/**
 * @file debug.h
 * @brief Debug System Module
 *
 * Handle semua debug panel dan zoom debug buat keperluan development.
 * Bisa nampilin info map, camera, player, collision, dll.
 */

#include "../lib/raylib/include/raylib.h"
#include <string>
#include <vector>

// ================================================================
// Debug Class
// Handle semua debug panel dan zoom debug
//
// Workflow:
// - Toggle() dipanggil tiap frame buat cek input TAB
// - Draw() jadi wrapper semua panel, hanya aktif kalau isDebugMode true
// - Tiap panel punya fungsi sendiri biar gampang di-maintain
// ================================================================

/*==============================================================================
 * Debug Class
 *==============================================================================*/

/**
 * @brief Class buat handle semua fungsi debug
 *
 * Nyediain debug panel untuk nampilin info real-time:
 * - Map info (ukuran, jumlah tile)
 * - Camera info (posisi, zoom)
 * - Player position
 * - Zoom control
 * - Frustum culling info
 * - Collision overlay
 */
class Debug
{
public:
    /**
     * @brief Handle toggle debug mode
     * @note Dipanggil tiap frame buat cek input TAB
     *       Otomatis nge-tracelog kalo mode berubah
     */
    void Toggle(void);

    /**
     * @brief Wrapper buat semua panel debug
     * @note Hanya render kalo isDebugMode true
     *       Manggil semua panel DrawXxxPanel()
     */
    void Draw(void);

    /**
     * @brief Overlay debug di world-space
     * @note Nampilin hitbox, collision, dan map bounds
     *       Langsung di-render ke world (bukan UI)
     */
    void DrawWorldOverlay(void);

private:
    /*==========================================================================
     * Internal Structs
     *==========================================================================*/

    /**
     * @brief Entry buat satu panel debug
     * @note Dipake buat ngelola panel yang aktif dan urutannya
     */
    struct DebugPanelEntry
    {
        std::string name;                        /**< Nama panel (buat judul) */
        void (Debug::*drawFn)(Rectangle bounds); /**< Pointer ke fungsi render panel */
        bool enabled;                            /**< Apakah panel ini aktif/ditampilin */
    };

    /*==========================================================================
     * Private Helper Methods
     *==========================================================================*/

    /**
     * @brief Hitung bounds (posisi & ukuran) panel berdasarkan index
     * @param index Urutan panel (0, 1, 2, ...)
     * @param panelWidth Lebar panel dalam pixel
     * @param panelHeight Tinggi panel dalam pixel
     * @return Rectangle posisi dan ukuran panel di layar
     */
    Rectangle GetPanelBounds(int index, float panelWidth, float panelHeight) const;

    /**
     * @brief Bikin daftar panel yang aktif
     * @return Vector berisi DebugPanelEntry yang enabled = true
     */
    std::vector<DebugPanelEntry> BuildActivePanels(void) const;

    /**
     * @brief Gambar overlay collision buat layer tertentu
     * @param layerName Nama layer collision yang mau digambar
     * @param rectColor Warna buat rectangle collision
     * @param polygonColor Warna buat polygon collision
     * @param pointColor Warna buat point collision
     */
    void DrawCollisionOverlay(const std::string &layerName, Color rectColor, Color polygonColor, Color pointColor);

    void DrawRaycastOverlay(void);

    /**
     * @brief Gambar frame/border panel debug
     * @param bounds Area panel
     * @param title Judul panel (ditampilin di atas)
     * @param borderColor Warna border panel
     */
    void DrawPanelFrame(Rectangle bounds, const char *title, Color borderColor) const;

    /*==========================================================================
     * Debug Panels
     *==========================================================================*/

    /**
     * @brief Panel info map
     * @note Nampilin ukuran map, jumlah tile, dll
     */
    void DrawMapPanel(Rectangle bounds);

    /**
     * @brief Panel info camera
     * @note Nampilin posisi camera, zoom level, dll
     */
    void DrawCameraPanel(Rectangle bounds);

    /**
     * @brief Panel info player position
     * @note Nampilin posisi player (world dan tile), direction, state
     */
    void DrawPlayerPanel(Rectangle bounds);

    /**
     * @brief Panel zoom debug + handle zoom input
     * @note Nampilin current zoom level dan handle input +/- buat zoom
     */
    void DrawZoomPanel(Rectangle bounds);

    /**
     * @brief Panel info frustum culling
     * @note Nampilin info tentang tile/camera yang lagi di-culling
     */
    void DrawFrustumPanel(Rectangle bounds);

    /**
     * @brief Panel info collision & world boundary
     * @note Nampilin status collision, boundary map, dll
     */
    void DrawCollisionPanel(Rectangle bounds);

    // void DebugMouse(GameState *state); // handle mouse position (sementara di-comment)
};

/*==============================================================================
 * Global Debug Instance
 *==============================================================================*/

/** Global instance debug - bisa diakses file lain pake extern */
extern Debug DebugInstance;

/** Flag buat toggle debug mode - didefinisiin di debugmode.cpp */
extern bool isDebugMode;