#pragma once
#include <raylib.h>

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
    void Toggle(); // handle TAB toggle + tracelog
    void Draw();   // wrapper semua panel

private:
    void DrawMapPanel();    // panel info map
    void DrawCameraPanel(); // panel info camera
    void DrawPlayerPanel(); // panel info player position
    void DrawZoomPanel();   // panel zoom debug + handle zoom input
    void DebugZoom(void);   // handle zoom input
    void DebugMouse(GameState *state); // handle mouse position
};

// global instance — bisa diakses file lain via extern
extern Debug DebugInstance;

// buat toggle debug mode, didefinisiin di debugmode.cpp
extern bool isDebugMode;