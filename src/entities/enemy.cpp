#include "enemy.h"
#include "player.h"
#include "map.h"
#include "../lib/raylib/include/raymath.h"
#include "debug.h"
#include "entities.h"
#include <cmath>

Enemy::Enemy()
{
    IsActive = true;
}

Enemy::~Enemy() {}

static EnemyDefinition MakeSlimeDefinition()
{
    return EnemyDefinition{
        .id = 0,
        .name = "Slime",
        .stats = {
            .maxHealth = 100.0f,
            .speed = 1.0f,
            .chaseSpeed = 1.5f,
            .damage = 5.0f,
            .baseDetectionRange = 120.0f,
            .chaseDetectionRange = 240.0f,
            .attackRange = 16.0f,
            .healthRegenRate = 10.0f,
            .healthRegenDelay = 2.0f,
            .patrolRadius = 128.0f,
            .turnBaseTriggerChance = 0.0f,
            .canTriggerTurnBased = false},
        .hitbox = {.size = {16.0f, 12.0f}, .offset = {8.0f, 14.0f}},
        .animSet = &SlimeAnimationSet};
}

static EnemyDefinition MakeSkeletonDefinition()
{
    return EnemyDefinition{
        .id = 1,
        .name = "Skeleton",
        .stats = {
            .maxHealth = 100.0f,
            .speed = 1.0f,
            .chaseSpeed = 1.75f,
            .damage = 10.0f,
            .baseDetectionRange = 120.0f,
            .chaseDetectionRange = 240.0f,
            .attackRange = 32.0f,
            .healthRegenRate = 10.0f,
            .healthRegenDelay = 2.0f,
            .patrolRadius = 128.0f,
            .turnBaseTriggerChance = 0.0f,
            .canTriggerTurnBased = false},
        .hitbox = {.size = {20.0f, 16.0f}, .offset = {6.0f, 12.0f}},
        .animSet = &SkeletonAnimationSet};
}

static EnemyDefinition MakeWolfDefinition()
{
    return EnemyDefinition{
        .id = 2,
        .name = "Wolf",
        .stats = {
            .maxHealth = 150.0f,
            .speed = 1.0f,
            .chaseSpeed = 2.5f,
            .damage = 20.0f,
            .baseDetectionRange = 120.0f,
            .chaseDetectionRange = 240.0f,
            .attackRange = 16.0f,
            .healthRegenRate = 10.0f,
            .healthRegenDelay = 2.0f,
            .patrolRadius = 128.0f,
            .turnBaseTriggerChance = 0.0f,
            .canTriggerTurnBased = false},
        .hitbox = {.size = {24.0f, 16.0f}, .offset = {4.0f, 12.0f}},
        .animSet = &WolfAnimationSet};
}

EnemyDefinition GetEnemyDefinition(const std::string &name)
{
    if (name == "Skeleton")
        return MakeSkeletonDefinition();
    if (name == "Wolf")
        return MakeWolfDefinition();
    return MakeSlimeDefinition();
}

void Enemy::Init(Vector2 pos, const char *name, int mapId, const EnemyDefinition &def)
{
    DefStorage = def;
    Def = &DefStorage;
    MapObjectID = mapId;
    SpawnPoint = pos;
    Name = name;

    // Assign dari definisi
    Health = def.stats.maxHealth;
    MaxHealth = def.stats.maxHealth;
    HitboxWidth = def.hitbox.size.x;
    HitboxHeight = def.hitbox.size.y;
    HitboxOffsetX = def.hitbox.offset.x;
    HitboxOffsetY = def.hitbox.offset.y;

    // Runtime state
    DetectionRange = def.stats.baseDetectionRange;
    HealthRegenTimer = 0.0f;
    PatrolTimer = 0.0f;
    AttackCooldownTimer = 0.0f;
    HitFlashTimer = 0.0f;
    KnockbackVelocity = {0, 0};
    DeathTimer = 0.0f;
    PlayerWasInRange = false;
    AIState = ENEMY_IDLE;

    Position.x = pos.x - (HitboxWidth / 2.0f) - HitboxOffsetX;
    Position.y = pos.y - (HitboxHeight / 2.0f) - HitboxOffsetY;
    PatrolTarget = pos;

    PlayAnimation(Anim, IDLE, DOWN, *Def->animSet);
    Anim.position = Position;
}

void Enemy::Update()
{
    if (!IsActive)
        return;

    if (Health <= 0)
    {
        if (Anim.state != DEAD)
        {
            PlayAnimation(Anim, DEAD, Anim.direction, *AnimSet);
            AIState = ENEMY_IDLE;
            DetectionRange = Def->stats.baseDetectionRange;
            Entities::RegisterDeath(GetCurrentMapPath(), MapObjectID);
        }

        DeathTimer += GetFrameTime();
        if (DeathTimer >= DeathDuration)
        {
            IsActive = false;
        }

        Anim.position = Position;
        UpdateAnimation(Anim, GetFrameTime());
        return;
    }

    if (HitFlashTimer > 0)
        HitFlashTimer -= GetFrameTime();
    if (AttackCooldownTimer > 0)
        AttackCooldownTimer -= GetFrameTime();

    if (Vector2Length(KnockbackVelocity) > 0.1f)
    {
        Vector2 move = Vector2Scale(KnockbackVelocity, GetFrameTime() * 60.0f);
        Vector2 nextX = {Position.x + move.x, Position.y};
        Vector2 nextY = {Position.x, Position.y + move.y};

        if (IsPositionSafe(nextX, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY))
            Position.x = nextX.x;
        if (IsPositionSafe(nextY, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY))
            Position.y = nextY.y;

        KnockbackVelocity = Vector2Scale(KnockbackVelocity, 0.85f);
    }
    else
    {
        KnockbackVelocity = {0, 0};
    }

    UpdateAI();

    Anim.position = Position;
    UpdateAnimation(Anim, GetFrameTime());
}

void Enemy::UpdateAI()
{
    if (!PlayerInstance.IsAlive())
    {
        if (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK)
        {
            AIState = ENEMY_IDLE;
            PlayAnimation(Anim, IDLE, Anim.direction, *AnimSet);
        }
        return;
    }

    DetectionRange = (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK)
                         ? Def->stats.chaseDetectionRange
                         : Def->stats.baseDetectionRange;

    if (HealthRegenTimer > 0)
    {
        HealthRegenTimer -= GetFrameTime();
    }
    else if (AIState != ENEMY_CHASE && AIState != ENEMY_ATTACK && Health < MaxHealth)
    {
        Health += Def->stats.healthRegenRate * GetFrameTime();
        if (Health > MaxHealth)
            Health = MaxHealth;
    }

    switch (AIState)
    {
    case ENEMY_IDLE:
        HandleIdle();
        break;
    case ENEMY_PATROL:
        HandlePatrol();
        break;
    case ENEMY_CHASE:
        HandleChase();
        break;
    case ENEMY_ATTACK:
        HandleAttack();
        break;
    case ENEMY_RETURN:
        HandleReturn();
        break;
    }
}

bool Enemy::CheckPlayerLoS()
{
    if (!tilesonMap || !PlayerInstance.IsAlive())
        return false;

    Vector2 enemyCenter = GetCenter();
    Vector2 playerCenter = PlayerInstance.GetCenter();

    float dist = Vector2Distance(enemyCenter, playerCenter);
    if (dist > DetectionRange)
        return false;

    Vector2 dir = Vector2Normalize(Vector2Subtract(playerCenter, enemyCenter));

    std::vector<MapObject> obstacles;
    for (auto &obj : tilesonMap->Objects)
    {
        if (obj.layerName == COLLISION_LAYER_NAME)
        {
            obstacles.push_back(obj);
        }
    }

    for (const auto &rect : DynamicObstacles)
    {
        MapObject dynObj;
        dynObj.bounds = rect;
        dynObj.hasPolygon = false;
        obstacles.push_back(dynObj);
    }

    RayHitResult hit = Ray.Cast(enemyCenter, dir, DetectionRange, obstacles);
    return (!hit.hit || hit.distance >= dist);
}

void Enemy::HandleIdle()
{
    if (CheckPlayerLoS())
    {
        AIState = ENEMY_CHASE;
        return;
    }

    PatrolTimer += GetFrameTime();
    if (PatrolTimer >= PatrolWaitTime)
    {
        PatrolTimer = 0;

        PatrolTarget = SpawnPoint; // fallback default
        for (int i = 0; i < 10; i++)
        {
            float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
            float r = (float)GetRandomValue(32, (int)Def->stats.patrolRadius);
            Vector2 potentialTarget = Vector2Add(SpawnPoint, {cosf(angle) * r, sinf(angle) * r});

            if (IsPositionSafe(potentialTarget, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY))
            {
                PatrolTarget = potentialTarget;
                break;
            }
        }

        AIState = ENEMY_PATROL;
        PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
    }
}

void Enemy::HandlePatrol()
{
    if (CheckPlayerLoS())
    {
        AIState = ENEMY_CHASE;
        return;
    }

    float dist = Vector2Distance(Position, PatrolTarget);
    if (dist < 10.0f)
    {
        AIState = ENEMY_IDLE;
        PlayAnimation(Anim, IDLE, Anim.direction, *AnimSet);
        return;
    }

    MoveTowards(PatrolTarget, Def->stats.speed);
    if (Anim.state != WALK)
        PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
}

void Enemy::HandleChase()
{
    if (AttackCooldownTimer > 0)
    {
        if (Anim.state != IDLE)
            PlayAnimation(Anim, IDLE, Anim.direction, *Def->animSet);
        return;
    }

    Vector2 enemyCenter = GetCenter();
    Vector2 playerCenter = PlayerInstance.GetCenter();
    float dist = Vector2Distance(enemyCenter, playerCenter);

    if (dist <= Def->stats.attackRange)
    {
        if (!PlayerWasInRange)
            PerformAttack();
        AIState = ENEMY_ATTACK;
        PlayerWasInRange = true;
        return;
    }

    PlayerWasInRange = false;

    if (dist > DetectionRange)
    {
        AIState = ENEMY_RETURN;
        PatrolTarget = SpawnPoint;
        PlayAnimation(Anim, WALK, Anim.direction, *Def->animSet);
        return;
    }

    MoveTowards(PlayerInstance.GetPosition(), Def->stats.chaseSpeed);
    if (Anim.state != WALK)
        PlayAnimation(Anim, WALK, Anim.direction, *Def->animSet);
}

void Enemy::HandleReturn()
{
    if (CheckPlayerLoS())
    {
        AIState = ENEMY_CHASE;
        return;
    }

    if (Vector2Distance(GetCenter(), SpawnPoint) < 5.0f)
    {
        AIState = ENEMY_IDLE;
        PlayAnimation(Anim, IDLE, Anim.direction, *Def->animSet);
        return;
    }

    MoveTowards(SpawnPoint, Def->stats.speed);
    if (Anim.state != WALK)
        PlayAnimation(Anim, WALK, Anim.direction, *Def->animSet);
}

void Enemy::HandleAttack()
{
    Vector2 enemyCenter = GetCenter();
    Vector2 playerCenter = PlayerInstance.GetCenter();

    float dist = Vector2Distance(enemyCenter, playerCenter);

    if (dist <= Def->stats.attackRange)
    {
        if (!PlayerWasInRange || AttackCooldownTimer <= 0)
            PerformAttack();
        PlayerWasInRange = true;
    }
    else
    {
        PlayerWasInRange = false;
        if (dist > Def->stats.attackRange * 1.2f)
        {
            AIState = ENEMY_CHASE;
            PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
        }
    }
}

void Enemy::PerformAttack()
{
    Vector2 enemyCenter = GetCenter();
    Vector2 playerCenter = PlayerInstance.GetCenter();
    Vector2 knockDir = Vector2Normalize(Vector2Subtract(playerCenter, enemyCenter));

    PlayerInstance.TakeDamage(Def->stats.damage, knockDir);

    PlayAnimation(Anim, ATTACK, Anim.direction, *AnimSet);
    Anim.isAttacking = true;
    AttackCooldownTimer = AttackCooldown;
}

void Enemy::TakeDamage(float amount, Vector2 knockback)
{
    Entity::TakeDamage(amount, knockback);
    HitFlashTimer = 0.15f;
    KnockbackVelocity = Vector2Scale(knockback, 6.0f);
    HealthRegenTimer = Def->stats.healthRegenDelay;
}

void Enemy::Render()
{
    if (!IsActive)
        return;

    DrawEllipse((int)Position.x + 16, (int)Position.y + 30, 10, 4, {0, 0, 0, 80});

    bool shouldDraw = true;
    if (Health <= 0)
    {
        float blinkFreq = (DeathTimer / DeathDuration) * 15.0f;
        shouldDraw = AnimEffects::ShouldBlink(DeathTimer, blinkFreq);
    }

    if (shouldDraw)
    {
        Color tint = WHITE;
        if (HitFlashTimer > 0)
            tint = RED;
        DrawAnimation(Anim, TEXTURE_ENEMIES, tint);
    }

    if (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK)
    {
        DrawRectangle((int)Position.x + 4, (int)Position.y + 38, 24, 4, BLACK);
        DrawRectangle((int)Position.x + 4, (int)Position.y + 38, (int)(24 * (Health / MaxHealth)), 4, RED);
    }

    if (isDebugMode)
    {
        Vector2 enemyCenter = GetCenter();
        DrawCircleLinesV(enemyCenter, DetectionRange, Fade(GRAY, 0.6f));
        DrawCircleLinesV(enemyCenter, Def->stats.attackRange, RED);
        Rectangle enemyHitbox = GetHitbox();
        DrawRectangleLinesEx(enemyHitbox, 1.0f, VIOLET);

        // Garis raycast ke player saat mengejar/menyerang
        if (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK)
        {
            DrawLineEx(enemyCenter, PlayerInstance.GetCenter(), 1.0f, RED);
        }
    }
}

/*==============================================================================
 * Global Utility Functions (definisi: src/enemy.cpp)
 *==============================================================================*/

void InitEnemy()
{
    LoadTileTexture(TEXTURE_ENEMIES, "assets/textures/enemies.png");
}

void SpawnRandomWave()
{
    int count = GetRandomValue(4, 7);
    for (int i = 0; i < count; i++)
        SpawnRandomEnemy();
}

void SpawnRandomEnemy()
{
    if (!tilesonMap) return;

    static const std::vector<std::string> enemyTypes = {"Slime", "Skeleton", "Wolf"};
    const std::string& picked = enemyTypes[GetRandomValue(0, 2)];
    EnemyDefinition def = GetEnemyDefinition(picked);

    Vector2 randomPos;
    bool validPos = false;
    float mapW = tilesonMap->width * 32.0f;
    float mapH = tilesonMap->height * 32.0f;

    for (int i = 0; i < 100; i++)
    {
        randomPos = {(float)GetRandomValue(32, (int)mapW - 32), (float)GetRandomValue(32, (int)mapH - 32)};
        if (IsPositionSafe(randomPos, def.hitbox.size.x, def.hitbox.size.y,
                           def.hitbox.offset.x, def.hitbox.offset.y))
        {
            validPos = true;
            break;
        }
    }

    if (validPos)
    {
        Enemy* en = new Enemy();
        en->Init(randomPos, picked.c_str(), -1, def);
        Entities::AddDynamic(en);
    }
}

void SaveEnemiesForMap(const std::string &mapPath) {}

bool LoadEnemiesForMap(const std::string &mapPath)
{
    return true;
}

void ClearEnemies()
{
    Entities::Clear();
}

// utility helper movement
void Enemy::MoveTowards(Vector2 target, float speed)
{
    Vector2 dir = Vector2Normalize(Vector2Subtract(target, Position));
    Vector2 move = Vector2Scale(dir, speed);

    if (IsPositionSafe({Position.x + move.x, Position.y}, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY))
        Position.x += move.x;
    if (IsPositionSafe({Position.x, Position.y + move.y}, HitboxWidth, HitboxHeight, HitboxOffsetX, HitboxOffsetY))
        Position.y += move.y;

    if (std::abs(dir.x) > std::abs(dir.y))
        Anim.direction = (dir.x > 0) ? RIGHT : LEFT;
    else
        Anim.direction = (dir.y > 0) ? DOWN : UP;
}
