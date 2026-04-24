#include "../include/enemy.h"
#include "../include/player.h"
#include "../include/map.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/debug.h"
#include <cmath>


Enemy::Enemy() {
    IsActive = true;
}

Enemy::~Enemy() {}

void Enemy::Init(Vector2 pos, const char* name, EnemyType type) {
    Type = type;
    
    // Tentukan AnimationSet berdasarkan tipe
    switch (Type) {
        case SLIME:    AnimSet = &SlimeAnimationSet; break;
        case SKELETON: AnimSet = &SkeletonAnimationSet; break;
        case WOLF:     AnimSet = &WolfAnimationSet; break;
        default:       AnimSet = &SlimeAnimationSet; break;
    }

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
    DetectionRange = BaseDetectionRange;
    PatrolTarget = pos;
    AttackCooldownTimer = 0.0f;
    PlayerWasInRange = false;
    
    // Inisialisasi animasi
    PlayAnimation(Anim, IDLE, DOWN, *AnimSet);
    Anim.position = Position;
}

void Enemy::Update() {
    if (!IsActive) return;

    // Handle kematian musuh
    if (Health <= 0) {
        if (Anim.state != DEAD) {
            PlayAnimation(Anim, DEAD, Anim.direction, *AnimSet);
            AIState = ENEMY_IDLE; // Reset AI state agar tidak lagi dianggap mengejar (memperbaiki bug raycast)
            DetectionRange = BaseDetectionRange; // Reset jangkauan pandangan ke standar
        }
        
        // Tetap update animasi agar frame kematian terlihat (jika ada)
        Anim.position = Position;
        UpdateAnimation(Anim, GetFrameTime());
        return;
    }

    if (AttackCooldownTimer > 0) AttackCooldownTimer -= GetFrameTime();

    UpdateAI();
    
    // Sinkronisasi posisi animasi dengan posisi entitas
    Anim.position = Position;
    UpdateAnimation(Anim, GetFrameTime());
}

void Enemy::UpdateAI() {
    // Jika Player mati, hentikan pengejaran/serangan
    if (!PlayerInstance.IsAlive()) {
        if (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK) {
            AIState = ENEMY_IDLE;
            PlayAnimation(Anim, IDLE, Anim.direction, *AnimSet);
            return;
        }
    }

    // Update jangkauan deteksi dinamis
    if (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK) {
        DetectionRange = ChaseDetectionRange;
    } else {
        DetectionRange = BaseDetectionRange;
    }

    // Health Regeneration instan saat IDLE atau PATROL
    if ((AIState == ENEMY_IDLE || AIState == ENEMY_PATROL) && Health < MaxHealth) {
        Health += HealthRegenRate * GetFrameTime();
        if (Health > MaxHealth) Health = MaxHealth;
    }

    switch (AIState) {
        case ENEMY_IDLE: HandleIdle(); break;
        case ENEMY_PATROL: HandlePatrol(); break;
        case ENEMY_CHASE: HandleChase(); break;
        case ENEMY_ATTACK: HandleAttack(); break;
    }
}

bool Enemy::CheckPlayerLoS() {
    if (!tilesonMap || !PlayerInstance.IsAlive()) return false;

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

        PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
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
        PlayAnimation(Anim, IDLE, Anim.direction, *AnimSet);
        return;
    }

    Vector2 dir = Vector2Normalize(Vector2Subtract(PatrolTarget, Position));
    Vector2 nextPos = Vector2Add(Position, Vector2Scale(dir, Speed));
    
    // Gunakan logic IsPositionSafe (sama seperti player) untuk menahan musuh
    if (IsPositionSafe(nextPos, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        Position = nextPos;
    }


    // Update arah animasi
    if (std::abs(dir.x) > std::abs(dir.y)) {
        Anim.direction = (dir.x > 0) ? RIGHT : LEFT;
    } else {
        Anim.direction = (dir.y > 0) ? DOWN : UP;
    }
    
    if (Anim.state != WALK) {
        PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
    }
}

void Enemy::HandleChase() {
    Vector2 playerPos = PlayerInstance.GetPosition();
    
    Vector2 enemyCenter = {
        Position.x + HitboxOffsetX + HitboxWidth / 2.0f,
        Position.y + HitboxOffsetY + HitboxHeight / 2.0f
    };
    Vector2 playerCenter = {
        playerPos.x + PlayerInstance.GetHitboxOffsetX() + PlayerInstance.GetHitboxWidth() / 2.0f,
        playerPos.y + PlayerInstance.GetHitboxOffsetY() + PlayerInstance.GetHitboxHeight() / 2.0f
    };

    float dist = Vector2Distance(enemyCenter, playerCenter);

    if (dist <= AttackRange) {
        if (!PlayerWasInRange) {
            PerformAttack();
        }
        AIState = ENEMY_ATTACK;
        PlayerWasInRange = true;
        return;
    } else {
        PlayerWasInRange = false;
    }

    // Jika pemain keluar dari Line of Sight atau terlalu jauh
    if (!CheckPlayerLoS()) {
        AIState = ENEMY_IDLE;
        PlayAnimation(Anim, IDLE, Anim.direction, *AnimSet);
        TraceLog(LOG_INFO, "ENEMY: Lost Player LoS. Returning to IDLE.");
        return;
    }

    Vector2 dir = Vector2Normalize(Vector2Subtract(playerPos, Position));
    Vector2 nextPos = Vector2Add(Position, Vector2Scale(dir, Speed * 1.5f));
    
    // Gunakan logic IsPositionSafe (sama seperti player) untuk menahan musuh
    if (IsPositionSafe(nextPos, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        Position = nextPos;
    }


    // Update arah animasi
    if (std::abs(dir.x) > std::abs(dir.y)) {
        Anim.direction = (dir.x > 0) ? RIGHT : LEFT;
    } else {
        Anim.direction = (dir.y > 0) ? DOWN : UP;
    }
    
    if (Anim.state != WALK) {
        PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
    }
}

void Enemy::HandleAttack() {
    Vector2 playerPos = PlayerInstance.GetPosition();
    
    Vector2 enemyCenter = {
        Position.x + HitboxOffsetX + HitboxWidth / 2.0f,
        Position.y + HitboxOffsetY + HitboxHeight / 2.0f
    };
    Vector2 playerCenter = {
        playerPos.x + PlayerInstance.GetHitboxOffsetX() + PlayerInstance.GetHitboxWidth() / 2.0f,
        playerPos.y + PlayerInstance.GetHitboxOffsetY() + PlayerInstance.GetHitboxHeight() / 2.0f
    };

    float dist = Vector2Distance(enemyCenter, playerCenter);
    
    if (dist <= AttackRange) {
        if (!PlayerWasInRange) {
            // Serangan instan saat pertama kali masuk (mengabaikan cooldown)
            PerformAttack();
        } else if (AttackCooldownTimer <= 0) {
            // Serangan berkala setelah berada di dalam area
            PerformAttack();
        }
        PlayerWasInRange = true;
    } else {
        PlayerWasInRange = false;
        
        if (dist > AttackRange * 1.2f) {
            AIState = ENEMY_CHASE;
            PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
            return;
        }
    }
}

void Enemy::PerformAttack() {
    PlayerInstance.TakeDamage(5.0f);
    TraceLog(LOG_INFO, "ENEMY: Hit Player! Remaining HP: %.1f", PlayerInstance.GetHealth());
    
    // Trigger animasi attack
    PlayAnimation(Anim, ATTACK, Anim.direction, *AnimSet);
    Anim.isAttacking = true;
    
    AttackCooldownTimer = AttackCooldown;
}

void Enemy::Render() {
    if (!IsActive) return;
    
    // Shadow sederhana
    DrawEllipse((int)Position.x + 16, (int)Position.y + 30, 10, 4, {0, 0, 0, 80});
    
    // Render musuh (menggunakan texture enemies yang baru)
    DrawAnimation(Anim, TEXTURE_ENEMIES);
    
    // Health Bar (Muncul hanya saat Chase/Attack dan diletakkan di bawah musuh)
    if (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK) {
        DrawRectangle((int)Position.x + 4.8, (int)Position.y + 38, 24, 4, BLACK);
        DrawRectangle((int)Position.x + 4.8, (int)Position.y + 38, (int)(24 * (Health / MaxHealth)), 4, RED);
    }
    

    // Fitur Debug Mode tambahan untuk Musuh
    if (isDebugMode) {
        Vector2 enemyCenter = {
            Position.x + HitboxOffsetX + HitboxWidth / 2.0f,
            Position.y + HitboxOffsetY + HitboxHeight / 2.0f
        };

        // 1. Jangkauan deteksi/chase (Lingkaran Abu-abu)
        DrawCircleLinesV(enemyCenter, DetectionRange, Fade(GRAY, 0.6f));

        // 2. Jangkauan serangan (Lingkaran Merah)
        DrawCircleLinesV(enemyCenter, AttackRange, RED);

        // 3. Garis Raycast saat mengejar atau mendeteksi Player
        if (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK) {
            Vector2 playerPos = PlayerInstance.GetPosition();
            Vector2 playerCenter = {
                playerPos.x + PlayerInstance.GetHitboxOffsetX() + PlayerInstance.GetHitboxWidth() / 2.0f,
                playerPos.y + PlayerInstance.GetHitboxOffsetY() + PlayerInstance.GetHitboxHeight() / 2.0f
            };
            DrawLineEx(enemyCenter, playerCenter, 1.0f, GREEN);
        }

        // 4. Hitbox Enemy (Rectangle Violet)
        Rectangle enemyHitbox = {
            Position.x + HitboxOffsetX,
            Position.y + HitboxOffsetY,
            HitboxWidth,
            HitboxHeight
        };
        DrawRectangleLinesEx(enemyHitbox, 1.0f, VIOLET);
        DrawText("Enemy", (int)enemyHitbox.x, (int)enemyHitbox.y - 12, 10, VIOLET);
    }
}

