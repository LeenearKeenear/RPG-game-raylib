#include "../include/enemy.h"
#include "../include/player.h"
#include "../include/map.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/debug.h"
#include "../include/entities.h"
#include <cmath>


Enemy::Enemy() {
    IsActive = true;
}

Enemy::~Enemy() {}

void Enemy::Init(Vector2 pos, const char* name, int mapId, EnemyType type, float radius) {
    Type = type;
    MapObjectID = mapId;
    SpawnPoint = pos; // Titik tengah spawn sebagai pusat patroli
    PatrolRadius = radius;
    
    // 1. Tentukan Konfigurasi (Hitbox & AnimSet) berdasarkan tipe
    switch (Type) {
        case SKELETON:
            AnimSet = &SkeletonAnimationSet;
            HitboxWidth = 20.0f; HitboxHeight = 16.0f;
            HitboxOffsetX = 6.0f; HitboxOffsetY = 12.0f;
            Health = 100.0f; MaxHealth = 100.0f;
            Damage = 10.0f;
            AttackRange = 32.0f;
            ChaseSpeed = 1.75f;
            break;
        case WOLF:
            AnimSet = &WolfAnimationSet;
            HitboxWidth = 24.0f; HitboxHeight = 16.0f;
            HitboxOffsetX = 4.0f; HitboxOffsetY = 12.0f;
            Health = 150.0f; MaxHealth = 150.0f;
            Damage = 20.0f;
            AttackRange = 16.0f;
            ChaseSpeed = 2.5f;
            break;
        case SLIME:
        default:
            AnimSet = &SlimeAnimationSet;
            HitboxWidth = 16.0f; HitboxHeight = 12.0f;
            HitboxOffsetX = 8.0f; HitboxOffsetY = 14.0f;
            Health = 100.0f; MaxHealth = 100.0f;
            Damage = 5.0f;
            AttackRange = 16.0f;
            ChaseSpeed = 1.5f;
            break;
    }

    // 2. Adjust Position agar titik TENGAH hitbox berada tepat di titik 'pos' (spawn point)
    Position.x = pos.x - (HitboxWidth / 2.0f) - HitboxOffsetX;
    Position.y = pos.y - (HitboxHeight / 2.0f) - HitboxOffsetY;

    // Validasi posisi setelah penyesuaian (opsional, tapi bagus untuk logging)
    if (!IsPositionSafe(pos, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        TraceLog(LOG_WARNING, "ENEMY: Spawn position (%.1f, %.1f) for '%s' is unsafe! Proceeding anyway...", pos.x, pos.y, name);
    }

    Name = name;
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
            AIState = ENEMY_IDLE;
            DetectionRange = BaseDetectionRange;
            
            // Daftarkan kematian seketika agar tetap tersimpan meski player langsung pindah map
            Entities::RegisterDeath(GetCurrentMapPath(), MapObjectID);
        }
        
        DeathTimer += GetFrameTime();
        if (DeathTimer >= DeathDuration) {
            IsActive = false; // Musuh benar-benar hilang setelah durasi mati selesai
        }

        // Tetap update animasi agar frame kematian terlihat (jika ada)
        Anim.position = Position;
        UpdateAnimation(Anim, GetFrameTime());
        return;
    }

    // Update Hit Flash Timer
    if (HitFlashTimer > 0) HitFlashTimer -= GetFrameTime();

    // Update Knockback
    if (Vector2Length(KnockbackVelocity) > 0.1f) {
        Vector2 move = Vector2Scale(KnockbackVelocity, GetFrameTime() * 60.0f);
        Vector2 nextX = { Position.x + move.x, Position.y };
        Vector2 nextY = { Position.x, Position.y + move.y };

        if (IsPositionSafe(nextX, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
            Position.x = nextX.x;
        }
        if (IsPositionSafe(nextY, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
            Position.y = nextY.y;
        }
        // Decay knockback
        KnockbackVelocity = Vector2Scale(KnockbackVelocity, 0.85f);
    } else {
        KnockbackVelocity = {0, 0};
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
        case ENEMY_RETURN: HandleReturn(); break;
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
        
        // Pilih titik patroli acak di sekitar TITIK SPAWN (bukan posisi sekarang)
        // Coba beberapa kali untuk mendapatkan posisi yang aman dan di dalam map
        for (int i = 0; i < 10; i++) {
            float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
            float r = (float)GetRandomValue(32, (int)PatrolRadius);
            Vector2 potentialTarget = Vector2Add(SpawnPoint, {cosf(angle) * r, sinf(angle) * r});
            
            // 1. Cek apakah aman (tidak menabrak tembok)
            // 2. Cek apakah di dalam batas map
            bool safe = IsPositionSafe(potentialTarget, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY);
            
            bool inMap = true;
            if (tilesonMap) {
                float mapW = tilesonMap->width * 32.0f;
                float mapH = tilesonMap->height * 32.0f;
                if (potentialTarget.x < 0 || potentialTarget.x > mapW || 
                    potentialTarget.y < 0 || potentialTarget.y > mapH) {
                    inMap = false;
                }
            }

            if (safe && inMap) {
                PatrolTarget = potentialTarget;
                break;
            }
            
            // Jika percobaan terakhir masih gagal, kembali ke titik spawn saja
            if (i == 9) PatrolTarget = SpawnPoint;
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
    Vector2 move = Vector2Scale(dir, Speed);
    Vector2 nextX = { Position.x + move.x, Position.y };
    Vector2 nextY = { Position.x, Position.y + move.y };

    if (IsPositionSafe(nextX, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        Position.x = nextX.x;
    }
    if (IsPositionSafe(nextY, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        Position.y = nextY.y;
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
    // Jika masih dalam cooldown serangan, jangan mengejar (diam dulu)
    if (AttackCooldownTimer > 0) {
        if (Anim.state != IDLE) {
            PlayAnimation(Anim, IDLE, Anim.direction, *AnimSet);
        }
        return;
    }

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
        AIState = ENEMY_RETURN; // Kembali ke titik spawn sebelum patroli lagi
        PatrolTarget = SpawnPoint;
        PlayAnimation(Anim, WALK, Anim.direction, *AnimSet); // Tetap animasi jalan
        TraceLog(LOG_INFO, "ENEMY: Lost Player. Returning to Spawn Point.");
        return;
    }

    Vector2 dir = Vector2Normalize(Vector2Subtract(playerPos, Position));
    Vector2 move = Vector2Scale(dir, ChaseSpeed);
    Vector2 nextX = { Position.x + move.x, Position.y };
    Vector2 nextY = { Position.x, Position.y + move.y };

    if (IsPositionSafe(nextX, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        Position.x = nextX.x;
    }
    if (IsPositionSafe(nextY, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        Position.y = nextY.y;
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

void Enemy::HandleReturn() {
    // Jika melihat pemain lagi saat kembali, langsung kejar
    if (CheckPlayerLoS()) {
        AIState = ENEMY_CHASE;
        TraceLog(LOG_INFO, "ENEMY: Spotted Player while returning! Chasing again.");
        return;
    }

    Vector2 enemyCenter = {
        Position.x + HitboxOffsetX + HitboxWidth / 2.0f,
        Position.y + HitboxOffsetY + HitboxHeight / 2.0f
    };

    float dist = Vector2Distance(enemyCenter, SpawnPoint);
    
    // Jika sudah cukup dekat dengan titik spawn, kembali ke IDLE
    if (dist < 5.0f) {
        AIState = ENEMY_IDLE;
        PlayAnimation(Anim, IDLE, Anim.direction, *AnimSet);
        TraceLog(LOG_INFO, "ENEMY: Returned to Spawn Point. Resuming IDLE.");
        return;
    }

    Vector2 dir = Vector2Normalize(Vector2Subtract(SpawnPoint, enemyCenter));
    Vector2 move = Vector2Scale(dir, Speed);
    Vector2 nextX = { Position.x + move.x, Position.y };
    Vector2 nextY = { Position.x, Position.y + move.y };

    if (IsPositionSafe(nextX, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        Position.x = nextX.x;
    } else {
        if (!IsPositionSafe(nextY, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
            AIState = ENEMY_IDLE;
            PlayAnimation(Anim, IDLE, Anim.direction, *AnimSet);
            return;
        }
    }

    if (IsPositionSafe(nextY, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        Position.y = nextY.y;
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

void Enemy::TakeDamage(float amount, Vector2 knockback) {
    Entity::TakeDamage(amount, knockback);
    
    // Trigger Hit Flash
    HitFlashTimer = 0.15f;
    
    // Trigger Knockback (Jangan timpa, tapi tambahkan jika perlu, atau set yang baru)
    KnockbackVelocity = Vector2Scale(knockback, 6.0f); // Intensitas knockback diperbesar
    
    TraceLog(LOG_INFO, "ENEMY [%p]: %s took %.1f damage. Remaining HP: %.1f", (void*)this, Name.c_str(), amount, Health);
}

void Enemy::PerformAttack() {
    // Hitung arah knockback ke arah player
    Vector2 enemyCenter = { Position.x + 16, Position.y + 16 };
    Vector2 playerPos = PlayerInstance.GetPosition();
    Vector2 playerCenter = { playerPos.x + 16, playerPos.y + 16 };
    Vector2 knockDir = Vector2Normalize(Vector2Subtract(playerCenter, enemyCenter));

    PlayerInstance.TakeDamage(Damage, knockDir);
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
    
    // Handle Blink saat mati
    bool shouldDraw = true;
    if (Health <= 0) {
        // Blink semakin cepat seiring waktu
        float blinkFreq = (DeathTimer / DeathDuration) * 15.0f;
        if ((int)(DeathTimer * blinkFreq * 10) % 2 == 0) {
            shouldDraw = false;
        }
    }

    if (shouldDraw) {
        // Tentukan warna tint (Hit Flash Merah)
        Color tint = WHITE;
        if (HitFlashTimer > 0) {
            tint = RED;
        }

        // Render musuh (menggunakan texture enemies yang baru)
        DrawAnimation(Anim, TEXTURE_ENEMIES, tint);
    }
    
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

