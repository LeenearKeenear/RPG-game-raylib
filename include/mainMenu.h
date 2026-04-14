#pragma once
#include "buttonTxt.h"
#include "screen.h"
#include <cstdint>

void InitMainMenu(GameState* state);
void UpdateMainMenu(GameState* state);
void RenderMainMenuToVirtualScreen(GameState* state);

enum MenuButton : std::uint8_t {
    BTN_START = 0,
    BTN_LOAD,
    BTN_OPTIONS,
    BTN_QUIT
};
