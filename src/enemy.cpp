#include "../include/enemy.h"
#include "../include/tiles.h"

Enemy::Enemy(Vector2 pos, std::string n, const AnimationSet &animSet) : position(pos), name(n) {
    health = 50.0f;
    maxHealth = 50.0f;
    
    anim.position = position;
    PlayAnimation(anim, IDLE, DOWN, animSet);
}

void Enemy::Update(float dt) {
    if (health <= 0) {
        if (!anim.isDead) {
            anim.isDead = true;
            if (anim.set) PlayAnimation(anim, DEAD, anim.direction, *anim.set);
        }
    } else {
        HandleAI(dt);
    }
    
    UpdateAnimation(anim, dt);
    anim.position = position;
}

void Enemy::Render() {
    DrawAnimation(anim, TEXTURE_ENEMY);
}

void Enemy::TakeDamage(float amount) {
    health -= amount;
    if (health < 0) health = 0;
}

void Enemy::HandleAI(float dt) {
    // Untuk sekarang hanya idle
}
