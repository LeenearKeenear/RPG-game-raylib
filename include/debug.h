#pragma once
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

class Debug
{
public:
    void Toggle(void); // handle TAB toggle + tracelog
    void Draw(void);   // wrapper semua panel
    void DrawWorldOverlay(void); // overlay debug di world-space: hitbox + collision + map bounds

private:
    struct DebugPanelEntry
    {
        std::string name;
        void (Debug::*drawFn)(Rectangle bounds);
        bool enabled;
    };

    // helper layout panel debug
    Rectangle GetPanelBounds(int index, float panelWidth, float panelHeight) const;
    std::vector<DebugPanelEntry> BuildActivePanels(void) const;
    void DrawPanelFrame(Rectangle bounds, const char *title, Color borderColor) const;

    void DrawMapPanel(Rectangle bounds);       // panel info map
    void DrawCameraPanel(Rectangle bounds);    // panel info camera
    void DrawPlayerPanel(Rectangle bounds);    // panel info player position
    void DrawZoomPanel(Rectangle bounds);      // panel zoom debug + handle zoom input
    void DrawFrustumPanel(Rectangle bounds);   // panel info frustum culling
    void DrawCollisionPanel(Rectangle bounds); // panel info collision & world boundary
    // void DebugMouse(GameState *state); // handle mouse position
};

// global instance — bisa diakses file lain via extern
extern Debug DebugInstance;

// buat toggle debug mode, didefinisikan di debugmode.cpp
extern bool isDebugMode;
