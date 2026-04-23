#pragma once

#include "../lib/raylib/include/raylib.h"
#include "animation.h"
#include <string>

class Enemy {
public:
    Enemy(Vector2 pos, std::string name, const AnimationSet &animSet);
    
    void Update(float dt);
    void Render();
    
    Vector2 GetPosition() const { return position; }
    std::string GetName() const { return name; }
    
    bool IsAlive() const { return health > 0; }
    void TakeDamage(float amount);

private:
    Vector2 position;
    std::string name;
    float health;
    float maxHealth;
    
    Animation anim;
    
    // AI simple
    void HandleAI(float dt);
};
