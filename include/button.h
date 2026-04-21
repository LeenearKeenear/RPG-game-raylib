#pragma once

/**
 * @file button.h
 * @brief Generic Button Template System
 *
 * Template-based button system yang support berbagai tipe content.
 *Policy-based design buat flexibility.
 */

#include "../lib/raylib/include/raylib.h"
#include <string>

/*==============================================================================
 * Button Policies
 *==============================================================================*/

/**
 * @brief Policy struct untuk text-based button
 * @note hold data dan behavior untuk tombol berbasis teks
 */
struct TextPolicy
{
    const char *text;
    int posX;
    int posY;
    int fontSize;
    Color textColor;
    int textWidth;
    float hoverAmount;

    TextPolicy() : text(nullptr), posX(0), posY(0), fontSize(0), textColor(BLANK), textWidth(0), hoverAmount(1.0F) {}

    TextPolicy(const char *text, int posX, int posY, int fontSize, Color color, float hover)
        : text(text), posX(posX), posY(posY), fontSize(fontSize), textColor(color), hoverAmount(hover)
    {
        textWidth = MeasureText(text, fontSize);
    }

    Rectangle GetBounds() const
    {
        return {static_cast<float>(posX), static_cast<float>(posY), 
                static_cast<float>(textWidth), static_cast<float>(fontSize)};
    }
};

/**
 * @brief Policy struct untuk image-based button
 * @note hold data dan behavior untuk tombol berbasis texture
 */
struct ImagePolicy
{
    Texture2D texture;
    Vector2 position;
    float hoverAmount;

    ImagePolicy() : texture({0}), position({0, 0}), hoverAmount(1.0F) {}

    ImagePolicy(const char *texturePath, Vector2 pos, float scale, float hover)
    {
        Image image = LoadImage(texturePath);
        int newWidth = static_cast<int>(image.width * scale);
        int newHeight = static_cast<int>(image.height * scale);
        ImageResize(&image, newWidth, newHeight);
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = pos;
        hoverAmount = hover;
    }

    void Unload()
    {
        UnloadTexture(texture);
    }

    Rectangle GetBounds() const
    {
        return {position.x, position.y, 
                static_cast<float>(texture.width), static_cast<float>(texture.height)};
    }
};

/*==============================================================================
 * Button Template Class
 *==============================================================================*/

/**
 * @brief Generic Button template class
 * @tparam PolicyType TextPolicy atau ImagePolicy
 *
 * Template class yang bisa handle berbagai tipe button content.
 * Gunakan policy buat define cara render dan collision detection.
 */
template<typename PolicyType>
class Button
{
public:
    Button() : policy() {}

    template<typename... Args>
    Button(Args... args) : policy(args...)
    {
    }

    ~Button()
    {
        if constexpr (std::is_same_v<PolicyType, ImagePolicy>)
        {
            policy.Unload();
        }
    }

    void Draw(Vector2 mousePosition)
    {
        Color currentColor = GetNormalColor();
        if (isHovered(mousePosition))
        {
            currentColor = DarkenColor(currentColor, policy.hoverAmount);
        }
        Render(currentColor);
    }

    [[nodiscard]] bool isClicked(Vector2 mousePosition, bool mouseClicked) const
    {
        return CheckCollisionPointRec(mousePosition, policy.GetBounds()) && mouseClicked;
    }

    [[nodiscard]] bool isHovered(Vector2 mousePosition) const
    {
        return CheckCollisionPointRec(mousePosition, policy.GetBounds());
    }

private:
    [[nodiscard]] Color GetNormalColor() const
    {
        if constexpr (std::is_same_v<PolicyType, TextPolicy>)
        {
            return policy.textColor;
        }
        else
        {
            return WHITE;
        }
    }

    void Render(Color color)
    {
        if constexpr (std::is_same_v<PolicyType, TextPolicy>)
        {
            DrawText(policy.text, policy.posX, policy.posY, policy.fontSize, color);
        }
        else
        {
            DrawTextureV(policy.texture, policy.position, color);
        }
    }

    static Color DarkenColor(Color color, float amount)
    {
        return {
            static_cast<unsigned char>(color.r * amount),
            static_cast<unsigned char>(color.g * amount),
            static_cast<unsigned char>(color.b * amount),
            color.a
        };
    }

    PolicyType policy;
};

/*==============================================================================
 * Type Aliases (Backward Compatibility)
 *==============================================================================*/

/**
 * @brief Text-based button - backward compatible dengan buttonTxt lama
 */
using buttonTxt = Button<TextPolicy>;

/**
 * @brief Image-based button - backward compatible dengan buttonImage lama
 */
using buttonImage = Button<ImagePolicy>;