#pragma once

/**
 * @file game_debug.h
 * @brief Debug System Module
 *
 * Header ini mendeklarasikan sistem debug untuk menampilkan
 * panel informasi runtime dan overlay debug di world space.
 */

#include "../lib/raylib/include/raylib.h"
#include "enemy.h"
#include <string>
#include <vector>

/*==============================================================================
 * Debug Class
 *==============================================================================*/

/**
 * @brief Menangani seluruh fitur panel dan overlay debug
 *
 * Sistem ini dipakai untuk menampilkan informasi runtime seperti:
 * - Info map
 * - Info camera
 * - Info player
 * - Info collision
 * - Info frustum culling
 * - Overlay debug di world space
 */
class Debug
{
public:
    /**
     * @brief Toggle debug mode berdasarkan input
     */
    void Toggle(void);

    /**
     * @brief Render seluruh panel debug yang aktif
     */
    void Draw(void);

    /**
     * @brief Render overlay debug langsung di world space
     */
    void DrawWorldOverlay(void);

    /** @brief Render overlay steering behavior untuk enemy */
    static void DrawSteeringOverlay(Enemy &enemy);

private:
    /*==========================================================================
     * Internal Structs
     *==========================================================================*/

    /**
     * @brief Menyimpan data satu panel debug
     */
    struct DebugPanelEntry
    {
        std::string name;                        // Nama panel
        void (Debug::*drawFn)(Rectangle bounds); // Pointer ke fungsi render panel
        bool enabled;                            // Status panel aktif atau tidak
    };

    /*==========================================================================
     * Private Helper Methods
     *==========================================================================*/

    /**
     * @brief Hitung posisi dan ukuran panel berdasarkan urutan
     * @param index Urutan panel
     * @param panelWidth Lebar panel
     * @param panelHeight Tinggi panel
     * @return Rectangle area panel di layar
     */
    Rectangle GetPanelBounds(int index, float panelWidth, float panelHeight) const;

    /** @brief Bangun daftar panel yang aktif */
    std::vector<DebugPanelEntry> BuildActivePanels(void) const;

    /**
     * @brief Gambar overlay collision untuk layer tertentu
     * @param layerName Nama layer collision
     * @param rectColor Warna rectangle collision
     * @param polygonColor Warna polygon collision
     * @param pointColor Warna titik polygon
     */
    void DrawCollisionOverlay(const std::string &layerName, Color rectColor, Color polygonColor, Color pointColor);

    /** @brief Render overlay raycast debug */
    void DrawRaycastOverlay(void);
    /** @brief Render overlay attack debug */
    void DrawAttackOverlay(void);
    /** @brief Render overlay enemy spawn debug */
    void DrawEnemySpawnOverlay(void);
    /** @brief Render overlay flow field debug */
    void DrawFlowFieldOverlay(const FlowField &field);

    /**
     * @brief Gambar frame panel debug
     * @param bounds Area panel
     * @param title Judul panel
     * @param borderColor Warna border panel
     */
    void DrawPanelFrame(Rectangle bounds, const char *title, Color borderColor) const;

    /*==========================================================================
     * Debug Panels
     *==========================================================================*/

    /**
     * @brief Gambar panel informasi map
     */
    void DrawMapPanel(Rectangle bounds);

    /**
     * @brief Gambar panel informasi camera
     */
    void DrawCameraPanel(Rectangle bounds);

    /**
     * @brief Gambar panel informasi player
     */
    void DrawPlayerPanel(Rectangle bounds);

    /**
     * @brief Gambar panel zoom debug
     */
    void DrawZoomPanel(Rectangle bounds);

    /**
     * @brief Gambar panel informasi frustum culling
     */
    void DrawFrustumPanel(Rectangle bounds);

    /**
     * @brief Gambar panel informasi collision dan boundary
     */
    void DrawCollisionPanel(Rectangle bounds);

    // void DebugMouse(GameState *state); // sementara tidak dipakai
};

/*==============================================================================
 * Global Debug Instance
 *==============================================================================*/

/** @brief Global instance debug */
extern Debug DebugInstance;

/** @brief Flag status debug mode */
extern bool isDebugMode;
/** @brief Flag toggle overlay flow field enemy */
extern bool showFlowFieldOverlay;
/** @brief Flag toggle overlay flow field player */
extern bool showFlowFieldOverlayPlayer;
