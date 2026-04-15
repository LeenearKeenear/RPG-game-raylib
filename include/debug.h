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
    void Toggle(void); // handle TAB toggle + tracelog
    void Draw(void);   // wrapper semua panel

private:
    void DrawMapPanel(void);    // panel info map
    void DrawCameraPanel(void); // panel info camera
    void DrawPlayerPanel(void); // panel info player position
    void DrawZoomPanel(void);   // panel zoom debug + handle zoom input
    // void DebugMouse(GameState *state); // handle mouse position
};

// global instance — bisa diakses file lain via extern
extern Debug DebugInstance;

// buat toggle debug mode, didefinisiin di debugmode.cpp
extern bool isDebugMode;