#include "../include/enemy.h"
#include "../include/player.h"
#include "../include/map.h"
#include "../lib/raylib/include/raymath.h"
#include <cmath>

Enemy::Enemy() {
    IsActive = true;
}

Enemy::~Enemy() {}

void Enemy::Init(Vector2 pos, const char* name) {
    // Validasi posisi sebelum inisialisasi
    if (!IsPositionSafe(pos, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        TraceLog(LOG_WARNING, "ENEMY: Spawn position (%.1f, %.1f) for '%s' is unsafe! Adjusting...", pos.x, pos.y, name);
        // Fallback sederhana: jika tidak aman, coba posisi player
        pos = PlayerInstance.GetPosition();
    }

    Position = pos;

    Name = name;
    Health = 50.0f;
    MaxHealth = 50.0f;
    AIState = ENEMY_IDLE;
    PatrolTarget = pos;
    
    // Inisialisasi animasi - menggunakan PlayerAnimationSet sebagai placeholder
    PlayAnimation(Anim, IDLE, DOWN, PlayerAnimationSet);
    Anim.position = Position;
}

void Enemy::Update() {
    if (!IsActive || Health <= 0) return;

    UpdateAI();
    
    // Sinkronisasi posisi animasi dengan posisi entitas
    Anim.position = Position;
    UpdateAnimation(Anim, GetFrameTime());
}

void Enemy::UpdateAI() {
    switch (AIState) {
        case ENEMY_IDLE: HandleIdle(); break;
        case ENEMY_PATROL: HandlePatrol(); break;
        case ENEMY_CHASE: HandleChase(); break;
        case ENEMY_ATTACK: HandleAttack(); break;
    }
}

bool Enemy::CheckPlayerLoS() {
    if (!tilesonMap) return false;

    Vector2 enemyCenter = {
        Position.x + HitboxOffsetX + HitboxWidth / 2.0f,
        Position.y + HitboxOffsetY + HitboxHeight / 2.0f
    };
    
    Vector2 playerPos = PlayerInstance.GetPosition();
    Vector2 playerCenter = {
        playerPos.x + PlayerInstance.GetHitboxOffsetX() + PlayerInstance.GetHitboxWidth() / 2.0f,
        playerPos.y + PlayerInstance.GetHitboxOffsetY() + PlayerInstance.GetHitboxHeight() / 2.0f
    };

    float dist = Vector2Distance(enemyCenter, playerCenter);
    if (dist > DetectionRange) return false;

    Vector2 dir = Vector2Normalize(Vector2Subtract(playerCenter, enemyCenter));
    
    // Ambil objek rintangan untuk pengecekan Raycast
    std::vector<MapObject> obstacles;
    for (auto &obj : tilesonMap->Objects) {
        if (obj.layerName == COLLISION_LAYER_NAME) {
            obstacles.push_back(obj);
        }
    }

    RayHitResult hit = Ray.Cast(enemyCenter, dir, DetectionRange, obstacles);
    
    // Jika tidak ada rintangan sebelum mencapai pemain, LoS aman
    if (!hit.hit || hit.distance >= dist) {
        return true;
    }

    return false;
}

void Enemy::HandleIdle() {
    if (CheckPlayerLoS()) {
        AIState = ENEMY_CHASE;
        TraceLog(LOG_INFO, "ENEMY: Spotted Player! Chasing...");
        return;
    }

    PatrolTimer += GetFrameTime();
    if (PatrolTimer >= PatrolWaitTime) {
        PatrolTimer = 0;
        
        // Pilih titik patroli acak di sekitar posisi sekarang
        // Coba beberapa kali untuk mendapatkan posisi yang aman
        for (int i = 0; i < 10; i++) {
            float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
            float radius = (float)GetRandomValue(48, 128);
            Vector2 potentialTarget = Vector2Add(Position, {cosf(angle) * radius, sinf(angle) * radius});
            
            if (IsPositionSafe(potentialTarget, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
                PatrolTarget = potentialTarget;
                break;
            }
            
            // Jika percobaan terakhir masih gagal, tetap di tempat
            if (i == 9) PatrolTarget = Position;
        }
        
        AIState = ENEMY_PATROL;

        PlayAnimation(Anim, WALK, Anim.direction, PlayerAnimationSet);
    }
}

void Enemy::HandlePatrol() {
    if (CheckPlayerLoS()) {
        AIState = ENEMY_CHASE;
        return;
    }

    float dist = Vector2Distance(Position, PatrolTarget);
    if (dist < 10.0f) {
        AIState = ENEMY_IDLE;
        PlayAnimation(Anim, IDLE, Anim.direction, PlayerAnimationSet);
        return;
    }

    Vector2 dir = Vector2Normalize(Vector2Subtract(PatrolTarget, Position));
    Position = Vector2Add(Position, Vector2Scale(dir, Speed));

    // Update arah animasi
    if (std::abs(dir.x) > std::abs(dir.y)) {
        Anim.direction = (dir.x > 0) ? RIGHT : LEFT;
    } else {
        Anim.direction = (dir.y > 0) ? DOWN : UP;
    }
    
    if (Anim.state != WALK) {
        PlayAnimation(Anim, WALK, Anim.direction, PlayerAnimationSet);
    }
}

void Enemy::HandleChase() {
    Vector2 playerPos = PlayerInstance.GetPosition();
    float dist = Vector2Distance(Position, playerPos);

    if (dist <= AttackRange) {
        AIState = ENEMY_ATTACK;
        PlayAnimation(Anim, ATTACK, Anim.direction, PlayerAnimationSet);
        return;
    }

    // Jika pemain terlalu jauh, berhenti mengejar
    if (dist > DetectionRange * 1.5f) {
        AIState = ENEMY_IDLE;
        PlayAnimation(Anim, IDLE, Anim.direction, PlayerAnimationSet);
        TraceLog(LOG_INFO, "ENEMY: Lost Player. Returning to IDLE.");
        return;
    }

    Vector2 dir = Vector2Normalize(Vector2Subtract(playerPos, Position));
    Position = Vector2Add(Position, Vector2Scale(dir, Speed * 1.5f));

    // Update arah animasi
    if (std::abs(dir.x) > std::abs(dir.y)) {
        Anim.direction = (dir.x > 0) ? RIGHT : LEFT;
    } else {
        Anim.direction = (dir.y > 0) ? DOWN : UP;
    }
    
    if (Anim.state != WALK) {
        PlayAnimation(Anim, WALK, Anim.direction, PlayerAnimationSet);
    }
}

void Enemy::HandleAttack() {
    Vector2 playerPos = PlayerInstance.GetPosition();
    float dist = Vector2Distance(Position, playerPos);
    
    if (dist > AttackRange * 1.2f) {
        AIState = ENEMY_CHASE;
        PlayAnimation(Anim, WALK, Anim.direction, PlayerAnimationSet);
        return;
    }

    // Logika serangan sederhana
    if (!Anim.isAttacking) {
        if (dist <= AttackRange) {
             PlayerInstance.SetHealth(PlayerInstance.GetHealth() - 5.0f);
             TraceLog(LOG_INFO, "ENEMY: Hit Player! Remaining HP: %.1f", PlayerInstance.GetHealth());
        }
        
        // Trigger animasi attack lagi
        PlayAnimation(Anim, ATTACK, Anim.direction, PlayerAnimationSet);
        Anim.isAttacking = true;
    }
}

void Enemy::Render() {
    if (!IsActive) return;
    
    // Shadow sederhana
    DrawEllipse((int)Position.x + 16, (int)Position.y + 28, 10, 4, {0, 0, 0, 80});
    
    // Render musuh (menggunakan texture ksatria sementara)
    DrawAnimation(Anim, TEXTURE_KNIGHT);
    
    // Health Bar
    DrawRectangle((int)Position.x + 4, (int)Position.y - 8, 24, 4, BLACK);
    DrawRectangle((int)Position.x + 4, (int)Position.y - 8, (int)(24 * (Health / MaxHealth)), 4, RED);
    
    // Debug info jika state bukan IDLE
    if (AIState == ENEMY_CHASE) {
        DrawText("!", (int)Position.x + 12, (int)Position.y - 22, 20, RED);
    }
}
