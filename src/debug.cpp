#include "../include/debug.h"
#include <raymath.h>

void Debug::Mouse(const Screen &Screen)
{
    Vector2 Mouse = GetMousePosition();
    float Scale = Screen.GetGameScale();
    float OffsetX = (Screen.GetWindowWidth() - (Screen::GAME_WIDTH * Scale)) * 0.5f;
    float OffsetY = (Screen.GetWindowHeight() - (Screen::GAME_HEIGHT * Scale)) * 0.5f;

    Vector2 VirtualMouse;
    VirtualMouse.x = (Mouse.x - OffsetX) / Scale;
    VirtualMouse.y = (Mouse.y - OffsetY) / Scale;
    VirtualMouse = Vector2Clamp(
        VirtualMouse,
        (Vector2){0, 0},
        (Vector2){(float)Screen::GAME_WIDTH, (float)Screen::GAME_HEIGHT});

    DrawRectangle(10, 10, 280, 55, WHITE);
    DrawText(
        TextFormat("Screen Mouse: [%i , %i]", (int)Mouse.x, (int)Mouse.y),
        15, 15, 20, BLACK);
    DrawText(
        TextFormat("Virtual Mouse: [%i , %i]", (int)VirtualMouse.x, (int)VirtualMouse.y),
        15, 40, 20, BLACK);
}