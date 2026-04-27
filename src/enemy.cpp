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
    SpawnPoint = pos;
    PatrolRadius = radius;
    
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

    Position.x = pos.x - (HitboxWidth / 2.0f) - HitboxOffsetX;
    Position.y = pos.y - (HitboxHeight / 2.0f) - HitboxOffsetY;

    Name = name;
    AIState = ENEMY_IDLE;
    DetectionRange = BaseDetectionRange;
    PatrolTarget = pos;
    AttackCooldownTimer = 0.0f;
    PlayerWasInRange = false;
    
    PlayAnimation(Anim, IDLE, DOWN, *AnimSet);
    Anim.position = Position;
}

void Enemy::Update() {
    if (!IsActive) return;

    if (Health <= 0) {
        if (Anim.state != DEAD) {
            PlayAnimation(Anim, DEAD, Anim.direction, *AnimSet);
            AIState = ENEMY_IDLE;
            DetectionRange = BaseDetectionRange;
            Entities::RegisterDeath(GetCurrentMapPath(), MapObjectID);
        }
        
        DeathTimer += GetFrameTime();
        if (DeathTimer >= DeathDuration) {
            IsActive = false; 
        }

        Anim.position = Position;
        UpdateAnimation(Anim, GetFrameTime());
        return;
    }

    if (HitFlashTimer > 0) HitFlashTimer -= GetFrameTime();
    if (AttackCooldownTimer > 0) AttackCooldownTimer -= GetFrameTime();

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
        KnockbackVelocity = Vector2Scale(KnockbackVelocity, 0.85f);
    } else {
        KnockbackVelocity = {0, 0};
    }

    UpdateAI();
    
    Anim.position = Position;
    UpdateAnimation(Anim, GetFrameTime());
}

void Enemy::UpdateAI() {
    if (!PlayerInstance.IsAlive()) {
        if (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK) {
            AIState = ENEMY_IDLE;
            PlayAnimation(Anim, IDLE, Anim.direction, *AnimSet);
            return;
        }
    }

    if (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK) {
        DetectionRange = ChaseDetectionRange;
    } else {
        DetectionRange = BaseDetectionRange;
    }

    if (HealthRegenTimer > 0)
    {
        HealthRegenTimer -= GetFrameTime();
    }
    else
    {
        if (AIState != ENEMY_CHASE && AIState != ENEMY_ATTACK && Health < MaxHealth)
        {
            Health += HealthRegenRate * GetFrameTime();
            if (Health > MaxHealth)
                Health = MaxHealth;
        }
    }

    switch (AIState) {
        case ENEMY_IDLE:   HandleIdle();   break;
        case ENEMY_PATROL: HandlePatrol(); break;
        case ENEMY_CHASE:  HandleChase();  break;
        case ENEMY_ATTACK: HandleAttack(); break;
        case ENEMY_RETURN: HandleReturn(); break;
    }
}

bool Enemy::CheckPlayerLoS() {
    if (!tilesonMap || !PlayerInstance.IsAlive()) return false;

    Vector2 enemyCenter = GetCenter();
    Vector2 playerCenter = PlayerInstance.GetCenter();

    float dist = Vector2Distance(enemyCenter, playerCenter);
    if (dist > DetectionRange) return false;

    Vector2 dir = Vector2Normalize(Vector2Subtract(playerCenter, enemyCenter));
    
    std::vector<MapObject> obstacles;
    for (auto &obj : tilesonMap->Objects) {
        if (obj.layerName == COLLISION_LAYER_NAME) {
            obstacles.push_back(obj);
        }
    }

    RayHitResult hit = Ray.Cast(enemyCenter, dir, DetectionRange, obstacles);
    return (!hit.hit || hit.distance >= dist);
}

void Enemy::HandleIdle() {
    if (CheckPlayerLoS()) {
        AIState = ENEMY_CHASE;
        return;
    }

    PatrolTimer += GetFrameTime();
    if (PatrolTimer >= PatrolWaitTime) {
        PatrolTimer = 0;
        
        for (int i = 0; i < 10; i++) {
            float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
            float r = (float)GetRandomValue(32, (int)PatrolRadius);
            Vector2 potentialTarget = Vector2Add(SpawnPoint, {cosf(angle) * r, sinf(angle) * r});
            
            if (IsPositionSafe(potentialTarget, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
                PatrolTarget = potentialTarget;
                break;
            }
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
    
    if (IsPositionSafe({ Position.x + move.x, Position.y }, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        Position.x += move.x;
    }
    if (IsPositionSafe({ Position.x, Position.y + move.y }, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        Position.y += move.y;
    }

    if (std::abs(dir.x) > std::abs(dir.y)) Anim.direction = (dir.x > 0) ? RIGHT : LEFT;
    else Anim.direction = (dir.y > 0) ? DOWN : UP;
    
    if (Anim.state != WALK) PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
}

void Enemy::HandleChase() {
    if (AttackCooldownTimer > 0) {
        if (Anim.state != IDLE) PlayAnimation(Anim, IDLE, Anim.direction, *AnimSet);
        return;
    }

    Vector2 playerPos = PlayerInstance.GetPosition();
    Vector2 enemyCenter = GetCenter();
    Vector2 playerCenter = PlayerInstance.GetCenter();

    float dist = Vector2Distance(enemyCenter, playerCenter);

    if (dist <= AttackRange) {
        if (!PlayerWasInRange) PerformAttack();
        AIState = ENEMY_ATTACK;
        PlayerWasInRange = true;
        return;
    } else {
        PlayerWasInRange = false;
    }

    if (dist > DetectionRange) {
        AIState = ENEMY_RETURN;
        PatrolTarget = SpawnPoint;
        PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
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

void Enemy::HandleAttack() {
    Vector2 enemyCenter = GetCenter();
    Vector2 playerCenter = PlayerInstance.GetCenter();

    float dist = Vector2Distance(enemyCenter, playerCenter);
    
    if (dist <= AttackRange) {
        if (!PlayerWasInRange || AttackCooldownTimer <= 0) PerformAttack();
        PlayerWasInRange = true;
    } else {
        PlayerWasInRange = false;
        if (dist > AttackRange * 1.2f) {
            AIState = ENEMY_CHASE;
            PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
        }
    }
}

void Enemy::HandleReturn() {
    if (CheckPlayerLoS()) {
        AIState = ENEMY_CHASE;
        return;
    }

    Vector2 enemyCenter = GetCenter();
    float dist = Vector2Distance(enemyCenter, SpawnPoint);
    
    if (dist < 5.0f) {
        AIState = ENEMY_IDLE;
        PlayAnimation(Anim, IDLE, Anim.direction, *AnimSet);
        return;
    }

    Vector2 dir = Vector2Normalize(Vector2Subtract(SpawnPoint, enemyCenter));
    Vector2 move = Vector2Scale(dir, Speed);
    
    if (IsPositionSafe({ Position.x + move.x, Position.y }, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        Position.x += move.x;
    }
    if (IsPositionSafe({ Position.x, Position.y + move.y }, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY)) {
        Position.y += move.y;
    }

    if (std::abs(dir.x) > std::abs(dir.y)) Anim.direction = (dir.x > 0) ? RIGHT : LEFT;
    else Anim.direction = (dir.y > 0) ? DOWN : UP;
    
    if (Anim.state != WALK) PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
}

void Enemy::TakeDamage(float amount, Vector2 knockback) {
    Entity::TakeDamage(amount, knockback);
    HitFlashTimer = 0.15f;
    KnockbackVelocity = Vector2Scale(knockback, 6.0f); 
    HealthRegenTimer = HealthRegenDelay; // Reset timer regen saat terkena damage
}

void Enemy::PerformAttack() {
    Vector2 enemyCenter = GetCenter();
    Vector2 playerCenter = PlayerInstance.GetCenter();
    Vector2 knockDir = Vector2Normalize(Vector2Subtract(playerCenter, enemyCenter));

    PlayerInstance.TakeDamage(Damage, knockDir);
    
    PlayAnimation(Anim, ATTACK, Anim.direction, *AnimSet);
    Anim.isAttacking = true;
    AttackCooldownTimer = AttackCooldown;
}

void Enemy::Render() {
    if (!IsActive) return;
    
    DrawEllipse((int)Position.x + 16, (int)Position.y + 30, 10, 4, {0, 0, 0, 80});
    
    bool shouldDraw = true;
    if (Health <= 0) {
        float blinkFreq = (DeathTimer / DeathDuration) * 15.0f;
        shouldDraw = AnimEffects::ShouldBlink(DeathTimer, blinkFreq);
    }

    if (shouldDraw) {
        Color tint = WHITE;
        if (HitFlashTimer > 0) tint = RED;
        DrawAnimation(Anim, TEXTURE_ENEMIES, tint);
    }
    
    if (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK) {
        DrawRectangle((int)Position.x + 4, (int)Position.y + 38, 24, 4, BLACK);
        DrawRectangle((int)Position.x + 4, (int)Position.y + 38, (int)(24 * (Health / MaxHealth)), 4, RED);
    }
    
    if (isDebugMode) {
        Vector2 enemyCenter = GetCenter();
        DrawCircleLinesV(enemyCenter, DetectionRange, Fade(GRAY, 0.6f));
        DrawCircleLinesV(enemyCenter, AttackRange, RED);
        Rectangle enemyHitbox = GetHitbox();
        DrawRectangleLinesEx(enemyHitbox, 1.0f, VIOLET);

        // Garis raycast ke player saat mengejar/menyerang
        if (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK) {
            DrawLineEx(enemyCenter, PlayerInstance.GetCenter(), 1.0f, RED);
        }
    }
}

/*==============================================================================
 * Global Utility Functions (definisi: src/enemy.cpp)
 *==============================================================================*/

int GetRandomDamage(int min, int max) {
    return GetRandomValue(min, max);
}

void InitEnemy() {
    LoadTileTexture(TEXTURE_ENEMIES, "texture/Enemies.png");
}

void InitEnemyTextures() {
    InitEnemy();
}

void SpawnRandomWave() {
    int count = GetRandomValue(4, 7);
    for (int i = 0; i < count; i++) SpawnRandomEnemy();
}

void SpawnRandomEnemy() {
    if (!tilesonMap) return;

    Vector2 randomPos;
    bool validPos = false;
    float mapW = tilesonMap->width * 32.0f;
    float mapH = tilesonMap->height * 32.0f;

    for (int i = 0; i < 100; i++) {
        randomPos = {(float)GetRandomValue(32, (int)mapW - 32), (float)GetRandomValue(32, (int)mapH - 32)};
        if (IsPositionSafe(randomPos, 16, 12, 8, 14)) {
            validPos = true;
            break;
        }
    }

    if (validPos) {
        EnemyType t = (EnemyType)GetRandomValue(0, 2);
        Enemy* en = new Enemy();
        en->Init(randomPos, "Random Enemy", -1, t);
        Entities::AddDynamic(en);
    }
}

void SaveEnemiesForMap(const std::string& mapPath) {
    // Entities::RegisterDeath already handles persistence of dead enemies
}

bool LoadEnemiesForMap(const std::string& mapPath) {
    return true; // Managed by map loading logic and Entities registry
}

void ClearEnemies() {
    Entities::Clear();
}
