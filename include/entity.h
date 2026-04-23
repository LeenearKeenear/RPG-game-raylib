#pragma once
#include "../lib/raylib/include/raylib.h"

class Entity
{
public:
    Vector2 Position = {0, 0};
    float Health = 100.0f;
    float MaxHealth = 100.0f;
    bool IsActive = true;

    virtual ~Entity() {}
    virtual void Update() = 0;
    virtual void Render() = 0;
    Vector2 GetPosition() const { return Position; }
    virtual bool IsAlive() const { return Health > 0; }
};
