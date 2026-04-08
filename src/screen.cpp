#include "../include/screen.h"
#define MIN(a, b) ((a) < (b) ? (a) : (b))

void Screen::Init()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(GAME_WIDTH, GAME_HEIGHT, "Dungeon Game");
    InitAudioDevice();

    WindowWidth = (int)(GetMonitorWidth(0) * SCALE_MULTIPLIER);
    WindowHeight = (int)(GetMonitorHeight(0) * SCALE_MULTIPLIER);

    GameScale = MIN(
        (float)WindowWidth / GAME_WIDTH,
        (float)WindowHeight / GAME_HEIGHT);

    SetWindowSize(WindowWidth, WindowHeight);
    SetWindowMinSize(
        (int)(GetMonitorWidth(0) * SCALE_MIN),
        (int)(GetMonitorHeight(0) * SCALE_MIN));

    SetTargetFPS(60);
}

void Screen::Update()
{
    WindowWidth = GetScreenWidth();
    WindowHeight = GetScreenHeight();
    GameScale = MIN(
        (float)WindowWidth / GAME_WIDTH,
        (float)WindowHeight / GAME_HEIGHT);
}

void Screen::Shutdown()
{
    CloseAudioDevice();
    CloseWindow();
}

float Screen::GetGameScale() const { return GameScale; }
int Screen::GetWindowWidth() const { return WindowWidth; }
int Screen::GetWindowHeight() const { return WindowHeight; }