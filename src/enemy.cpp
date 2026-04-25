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

/**
 * Menginisialisasi statistik musuh dan konfigurasi visual berdasarkan tipenya.
 */
void Enemy::Init(Vector2 pos, const char* name, int mapId, EnemyType type, float radius) {
    Type = type;
    MapObjectID = mapId;
    SpawnPoint = pos; // Menggunakan titik spawn sebagai pusat area patroli
    PatrolRadius = radius;
    
    // 1. Konfigurasi Statistik, Hitbox, dan Set Animasi berdasarkan Tipe Musuh
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

    // 2. Sesuaikan posisi agar titik TENGAH hitbox sejajar dengan titik spawn 'pos'
    Position.x = pos.x - (HitboxWidth / 2.0f) - HitboxOffsetX;
    Position.y = pos.y - (HitboxHeight / 2.0f) - HitboxOffsetY;

    if (!IsPositionSafe(pos, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        TraceLog(LOG_WARNING, "ENEMY: Posisi spawn (%.1f, %.1f) untuk '%s' tidak aman! Tetap dilanjutkan...", pos.x, pos.y, name);
    }

    Name = name;
    AIState = ENEMY_IDLE;
    DetectionRange = BaseDetectionRange;
    PatrolTarget = pos;
    AttackCooldownTimer = 0.0f;
    PlayerWasInRange = false;
    
    PlayAnimation(Anim, IDLE, DOWN, *AnimSet);
    Anim.position = Position;
}

/**
 * Loop update utama untuk musuh.
 * Menangani lifecycle (kematian), fisika (knockback), dan logika AI.
 */
void Enemy::Update() {
    if (!IsActive) return;

    // Menangani Logika Kematian
    if (Health <= 0) {
        if (Anim.state != DEAD) {
            PlayAnimation(Anim, DEAD, Anim.direction, *AnimSet);
            AIState = ENEMY_IDLE;
            DetectionRange = BaseDetectionRange;
            
            // Mencatat status kematian di registri map
            Entities::RegisterDeath(GetCurrentMapPath(), MapObjectID);
        }
        
        DeathTimer += GetFrameTime();
        if (DeathTimer >= DeathDuration) {
            IsActive = false; // Entitas dihapus setelah animasi kematian/fading selesai
        }

        Anim.position = Position;
        UpdateAnimation(Anim, GetFrameTime());
        return;
    }

    // Memperbarui Timer
    if (HitFlashTimer > 0) HitFlashTimer -= GetFrameTime();
    if (AttackCooldownTimer > 0) AttackCooldownTimer -= GetFrameTime();

    // Memproses Fisika Knockback
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
        KnockbackVelocity = Vector2Scale(KnockbackVelocity, 0.85f); // Gesekan
    } else {
        KnockbackVelocity = {0, 0};
    }

    UpdateAI();
    
    Anim.position = Position;
    UpdateAnimation(Anim, GetFrameTime());
}

/**
 * Dispatcher Mesin Status (FSM) AI.
 */
void Enemy::UpdateAI() {
    // Berhenti agresif jika pemain mati
    if (!PlayerInstance.IsAlive()) {
        if (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK) {
            AIState = ENEMY_IDLE;
            PlayAnimation(Anim, IDLE, Anim.direction, *AnimSet);
            return;
        }
    }

    // Jarak Deteksi Dinamis: Perbesar jarak saat pemain terlihat untuk mencegah "leashing" yang terlalu mudah
    if (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK) {
        DetectionRange = ChaseDetectionRange;
    } else {
        DetectionRange = BaseDetectionRange;
    }

    // Regenerasi Kesehatan Pasif saat status non-tempur
    if ((AIState == ENEMY_IDLE || AIState == ENEMY_PATROL) && Health < MaxHealth) {
        Health += HealthRegenRate * GetFrameTime();
        if (Health > MaxHealth) Health = MaxHealth;
    }

    switch (AIState) {
        case ENEMY_IDLE:   HandleIdle();   break;
        case ENEMY_PATROL: HandlePatrol(); break;
        case ENEMY_CHASE:  HandleChase();  break;
        case ENEMY_ATTACK: HandleAttack(); break;
        case ENEMY_RETURN: HandleReturn(); break;
    }
}

/**
 * Menjalankan raycast untuk memeriksa apakah pemain terlihat dan berada dalam jangkauan.
 */
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
    
    // Periksa tabrakan tembok di sepanjang garis menuju pemain
    std::vector<MapObject> obstacles;
    for (auto &obj : tilesonMap->Objects) {
        if (obj.layerName == COLLISION_LAYER_NAME) {
            obstacles.push_back(obj);
        }
    }

    RayHitResult hit = Ray.Cast(enemyCenter, dir, DetectionRange, obstacles);
    
    // Jika tidak ada rintangan yang terkena, atau rintangan lebih jauh dari pemain, LoS bersih
    return (!hit.hit || hit.distance >= dist);
}

/**
 * Status Idle: Menunggu timer atau deteksi pemain.
 */
void Enemy::HandleIdle() {
    if (CheckPlayerLoS()) {
        AIState = ENEMY_CHASE;
        TraceLog(LOG_INFO, "ENEMY: Melihat Pemain! Mengejar...");
        return;
    }

    PatrolTimer += GetFrameTime();
    if (PatrolTimer >= PatrolWaitTime) {
        PatrolTimer = 0;
        
        // Pilih titik patroli acak dalam PatrolRadius dari TITIK SPAWN
        for (int i = 0; i < 10; i++) {
            float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
            float r = (float)GetRandomValue(32, (int)PatrolRadius);
            Vector2 potentialTarget = Vector2Add(SpawnPoint, {cosf(angle) * r, sinf(angle) * r});
            
            bool safe = IsPositionSafe(potentialTarget, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY);
            bool inMap = true;
            if (tilesonMap) {
                float mapW = tilesonMap->width * 32.0f;
                float mapH = tilesonMap->height * 32.0f;
                if (potentialTarget.x < 0 || potentialTarget.x > mapW || 
                    potentialTarget.y < 0 || potentialTarget.y > mapH) inMap = false;
            }

            if (safe && inMap) {
                PatrolTarget = potentialTarget;
                break;
            }
            if (i == 9) PatrolTarget = SpawnPoint; // Kembali ke spawn jika gagal menemukan titik aman
        }
        
        AIState = ENEMY_PATROL;
        PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
    }
}

/**
 * Status Patroli: Bergerak menuju target yang dipilih secara acak.
 */
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
    
    // Mencoba bergerak di sumbu X dan Y secara independen (Sliding movement)
    if (IsPositionSafe({ Position.x + move.x, Position.y }, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        Position.x += move.x;
    }
    if (IsPositionSafe({ Position.x, Position.y + move.y }, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        Position.y += move.y;
    }

    // Perbarui arah animasi
    if (std::abs(dir.x) > std::abs(dir.y)) Anim.direction = (dir.x > 0) ? RIGHT : LEFT;
    else Anim.direction = (dir.y > 0) ? DOWN : UP;
    
    if (Anim.state != WALK) PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
}

/**
 * Status Chase (Mengejar): Mengikuti pemain secara terus-menerus.
 */
void Enemy::HandleChase() {
    // Jeda serangan: Jangan bergerak saat timer cooldown serangan aktif
    if (AttackCooldownTimer > 0) {
        if (Anim.state != IDLE) PlayAnimation(Anim, IDLE, Anim.direction, *AnimSet);
        return;
    }

    Vector2 playerPos = PlayerInstance.GetPosition();
    Vector2 enemyCenter = { Position.x + HitboxOffsetX + HitboxWidth / 2.0f, Position.y + HitboxOffsetY + HitboxHeight / 2.0f };
    Vector2 playerCenter = { playerPos.x + PlayerInstance.GetHitboxOffsetX() + PlayerInstance.GetHitboxWidth() / 2.0f, playerPos.y + PlayerInstance.GetHitboxOffsetY() + PlayerInstance.GetHitboxHeight() / 2.0f };

    float dist = Vector2Distance(enemyCenter, playerCenter);

    // Transisi ke serangan jika sudah cukup dekat
    if (dist <= AttackRange) {
        if (!PlayerWasInRange) PerformAttack();
        AIState = ENEMY_ATTACK;
        PlayerWasInRange = true;
        return;
    } else {
        PlayerWasInRange = false;
    }

    // Kehilangan target jika di luar jangkauan
    if (dist > DetectionRange) {
        AIState = ENEMY_RETURN;
        PatrolTarget = SpawnPoint;
        PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
        TraceLog(LOG_INFO, "ENEMY: Kehilangan Pemain (Di luar jangkauan). Kembali ke Titik Spawn.");
        return;
    }

    Vector2 dir = Vector2Normalize(Vector2Subtract(playerPos, Position));
    Vector2 move = Vector2Scale(dir, ChaseSpeed);
    
    if (IsPositionSafe({ Position.x + move.x, Position.y }, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) Position.x += move.x;
    if (IsPositionSafe({ Position.x, Position.y + move.y }, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) Position.y += move.y;

    if (std::abs(dir.x) > std::abs(dir.y)) Anim.direction = (dir.x > 0) ? RIGHT : LEFT;
    else Anim.direction = (dir.y > 0) ? DOWN : UP;
    
    if (Anim.state != WALK) PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
}

/**
 * Status Serangan: Melakukan serangan terhadap pemain.
 */
void Enemy::HandleAttack() {
    Vector2 playerPos = PlayerInstance.GetPosition();
    Vector2 enemyCenter = { Position.x + HitboxOffsetX + HitboxWidth / 2.0f, Position.y + HitboxOffsetY + HitboxHeight / 2.0f };
    Vector2 playerCenter = { playerPos.x + PlayerInstance.GetHitboxOffsetX() + PlayerInstance.GetHitboxWidth() / 2.0f, playerPos.y + PlayerInstance.GetHitboxOffsetY() + PlayerInstance.GetHitboxHeight() / 2.0f };

    float dist = Vector2Distance(enemyCenter, playerCenter);
    
    if (dist <= AttackRange) {
        if (!PlayerWasInRange || AttackCooldownTimer <= 0) PerformAttack();
        PlayerWasInRange = true;
    } else {
        PlayerWasInRange = false;
        // Buffer histeresis untuk mencegah perubahan status yang berulang-ulang (flickering)
        if (dist > AttackRange * 1.2f) {
            AIState = ENEMY_CHASE;
            PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
            return;
        }
    }
}

/**
 * Status Return (Kembali): Bergerak kembali ke titik spawn setelah kehilangan pemain.
 */
void Enemy::HandleReturn() {
    if (CheckPlayerLoS()) {
        AIState = ENEMY_CHASE;
        TraceLog(LOG_INFO, "ENEMY: Melihat Pemain saat kembali! Mengejar lagi.");
        return;
    }

    Vector2 enemyCenter = { Position.x + HitboxOffsetX + HitboxWidth / 2.0f, Position.y + HitboxOffsetY + HitboxHeight / 2.0f };
    float dist = Vector2Distance(enemyCenter, SpawnPoint);
    
    if (dist < 5.0f) {
        AIState = ENEMY_IDLE;
        PlayAnimation(Anim, IDLE, Anim.direction, *AnimSet);
        TraceLog(LOG_INFO, "ENEMY: Telah kembali ke Titik Spawn. Melanjutkan IDLE.");
        return;
    }

    Vector2 dir = Vector2Normalize(Vector2Subtract(SpawnPoint, enemyCenter));
    Vector2 move = Vector2Scale(dir, Speed);
    
    if (IsPositionSafe({ Position.x + move.x, Position.y }, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        Position.x += move.x;
    } else {
        // Jika terjebak saat kembali, cukup beralih ke status idle
        if (!IsPositionSafe({ Position.x, Position.y + move.y }, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
            AIState = ENEMY_IDLE;
            PlayAnimation(Anim, IDLE, Anim.direction, *AnimSet);
            return;
        }
    }

    if (IsPositionSafe({ Position.x, Position.y + move.y }, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) Position.y += move.y;

    if (std::abs(dir.x) > std::abs(dir.y)) Anim.direction = (dir.x > 0) ? RIGHT : LEFT;
    else Anim.direction = (dir.y > 0) ? DOWN : UP;
    
    if (Anim.state != WALK) PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
}

/**
 * Menangani damage yang diterima oleh musuh.
 */
void Enemy::TakeDamage(float amount, Vector2 knockback) {
    Entity::TakeDamage(amount, knockback);
    HitFlashTimer = 0.15f;
    KnockbackVelocity = Vector2Scale(knockback, 6.0f); 
    
    TraceLog(LOG_INFO, "ENEMY [%p]: %s menerima %.1f damage. Sisa HP: %.1f", (void*)this, Name.c_str(), amount, Health);
}

/**
 * Menjalankan aksi yang memberikan damage kepada pemain.
 */
void Enemy::PerformAttack() {
    Vector2 enemyCenter = { Position.x + 16, Position.y + 16 };
    Vector2 playerPos = PlayerInstance.GetPosition();
    Vector2 playerCenter = { playerPos.x + 16, playerPos.y + 16 };
    Vector2 knockDir = Vector2Normalize(Vector2Subtract(playerCenter, enemyCenter));

    PlayerInstance.TakeDamage(Damage, knockDir);
    TraceLog(LOG_INFO, "ENEMY: Memukul Pemain! Sisa HP: %.1f", PlayerInstance.GetHealth());
    
    PlayAnimation(Anim, ATTACK, Anim.direction, *AnimSet);
    Anim.isAttacking = true;
    AttackCooldownTimer = AttackCooldown;
}

/**
 * Me-render sprite musuh, health bar, dan overlay debug.
 */
void Enemy::Render() {
    if (!IsActive) return;
    
    // Shadow sederhana
    DrawEllipse((int)Position.x + 16, (int)Position.y + 30, 10, 4, {0, 0, 0, 80});
    
    // Efek berkedip saat mendekati kematian (atau saat animasi kematian)
    bool shouldDraw = true;
    if (Health <= 0) {
        float blinkFreq = (DeathTimer / DeathDuration) * 15.0f;
        if ((int)(DeathTimer * blinkFreq * 10) % 2 == 0) shouldDraw = false;
    }

    if (shouldDraw) {
        Color tint = WHITE;
        if (HitFlashTimer > 0) tint = RED; // Menerapkan flash Merah saat terkena hit
        DrawAnimation(Anim, TEXTURE_ENEMIES, tint);
    }
    
    // Health Bar Melayang (Hanya terlihat saat sedang agresif)
    if (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK) {
        DrawRectangle((int)Position.x + 4.8, (int)Position.y + 38, 24, 4, BLACK);
        DrawRectangle((int)Position.x + 4.8, (int)Position.y + 38, (int)(24 * (Health / MaxHealth)), 4, RED);
    }
    
    // --- Visualisasi Debug ---
    if (isDebugMode) {
        Vector2 enemyCenter = { Position.x + HitboxOffsetX + HitboxWidth / 2.0f, Position.y + HitboxOffsetY + HitboxHeight / 2.0f };

        // 1. Radius Deteksi/Chase (Abu-abu)
        DrawCircleLinesV(enemyCenter, DetectionRange, Fade(GRAY, 0.6f));

        // 2. Radius Serangan (Merah)
        DrawCircleLinesV(enemyCenter, AttackRange, RED);

        // 3. Raycast Line-of-Sight (Hijau)
        if (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK) {
            Vector2 playerPos = PlayerInstance.GetPosition();
            Vector2 playerCenter = { playerPos.x + PlayerInstance.GetHitboxOffsetX() + PlayerInstance.GetHitboxWidth() / 2.0f, playerPos.y + PlayerInstance.GetHitboxOffsetY() + PlayerInstance.GetHitboxHeight() / 2.0f };
            DrawLineEx(enemyCenter, playerCenter, 1.0f, GREEN);
        }

        // 4. Hitbox Tabrakan (Violet)
        Rectangle enemyHitbox = { Position.x + HitboxOffsetX, Position.y + HitboxOffsetY, HitboxWidth, HitboxHeight };
        DrawRectangleLinesEx(enemyHitbox, 1.0f, VIOLET);
        DrawText("Enemy", (int)enemyHitbox.x, (int)enemyHitbox.y - 12, 10, VIOLET);
    }
}

