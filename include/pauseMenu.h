#pragma once

#include "../lib/raylib/include/raylib.h"
#include "buttonTxt.h"
#include "screen.h"
#include <array>
#include <cstdint>

class PauseMenu {
public:
    PauseMenu();
    ~PauseMenu();

    void Show();
    void Hide();
    bool IsActive() const;

    void Update(GameState* state, Vector2 mousePosition, bool mouseClicked);
    void Draw(Vector2 mousePosition);

private:
    void CalculateDimensions();
    void HandleButtonClick(int buttonIndex, GameState* state);

    bool active;
    std::array<buttonTxt, 5> buttons;
    std::array<const char*, 5> buttonTexts;

    Vector2 position;
    int width;
    int height;
    Rectangle backgroundRect;
};