#include "enemy.h"
#include "screen.h"
#include "enemy_ai.h"
#include "player.h"
#include "map.h"
#include "datadriven.h"
#include "../lib/raylib/include/raymath.h"
#include "../lib/json/include/nlohmann/json.hpp"
#include "debug.h"
#include "entities.h"
#include <cmath>
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;
using namespace DataDriven;

EnemyDataManager enemyData;

/*==============================================================================
 * EnemyDataManager
 *==============================================================================*/

void EnemyDataManager::Load(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Cannot open: " + path);

    json root = json::parse(file);

    for (auto &[name, data] : root.at("enemies").items())
    {
        EnemyDefinition def;
        def.id = SafeGet<int>(data, "id", -1);
        def.name = name;

        const auto &s = data.at("stats");
        def.stats.maxHealth = SafeGet<float>(s, "maxHealth", 100.f);
        def.stats.speed = SafeGet<float>(s, "speed", 1.f);
        def.stats.chaseSpeed = SafeGet<float>(s, "chaseSpeed", 1.5f);
        def.stats.damage = SafeGet<float>(s, "damage", 10.f);
        def.stats.baseDetectionRange = SafeGet<float>(s, "baseDetectionRange", 120.f);
        def.stats.chaseDetectionRange = SafeGet<float>(s, "chaseDetectionRange", 240.f);
        def.stats.attackRange = SafeGet<float>(s, "attackRange", 32.f);
        def.stats.healthRegenRate = SafeGet<float>(s, "healthRegenRate", 10.f);
        def.stats.healthRegenDelay = SafeGet<float>(s, "healthRegenDelay", 1.f);
        def.stats.patrolRadius = SafeGet<float>(s, "patrolRadius", 130.f);
        def.stats.turnBaseTriggerChance = SafeGet<float>(s, "turnBaseTriggerChance", 0.f);
        def.stats.canTriggerTurnBased = SafeGet<bool>(s, "canTriggerTurnBased", false);

        const auto &h = data.at("hitbox");
        def.hitbox.size = ParseVector2(h.at("size"));
        def.hitbox.offset = ParseVector2(h.at("offset"));

        def.animSet = ResolveAnimSet(name);

        definitions_[name] = std::move(def);
    }
}

const EnemyDefinition &EnemyDataManager::Get(const std::string &name) const
{
    auto it = definitions_.find(name);
    if (it == definitions_.end())
        throw std::runtime_error("EnemyDefinition not found: " + name);
    return it->second;
}

std::vector<std::string> EnemyDataManager::GetAllNames() const
{
    std::vector<std::string> names;
    names.reserve(definitions_.size());
    for (auto &[k, _] : definitions_)
        names.push_back(k);
    return names;
}

/*==============================================================================
 * Enemy — Lifecycle
 *==============================================================================*/

Enemy::Enemy()
{
    IsActive = true;
}

Enemy::~Enemy() {}

/**
 * @brief Inisialisasi enemy dari definisi yang sudah dimuat.
 * @param pos Posisi spawn di world space
 * @param name Nama enemy (harus cocok dengan key di enemies.json)
 * @param mapId ID object di Tiled, dipakai untuk RegisterDeath
 * @param def Definisi enemy yang sudah dimuat dari EnemyDataManager
 */
void Enemy::Init(Vector2 pos, const char *name, int mapId, const EnemyDefinition &def)
{
    DefStorage = def;
    Def = &DefStorage;
    AnimSet = Def->animSet;
    MapObjectID = mapId;
    SpawnPoint = pos;
    Name = name;

    Health = def.stats.maxHealth;
    MaxHealth = def.stats.maxHealth;
    HitboxWidth = def.hitbox.size.x;
    HitboxHeight = def.hitbox.size.y;
    HitboxOffsetX = def.hitbox.offset.x;
    HitboxOffsetY = def.hitbox.offset.y;

    // Runtime state — reset setiap spawn
    DetectionRange = def.stats.baseDetectionRange;
    HealthRegenTimer = 0.0f;
    PatrolTimer = 0.0f;
    AttackCooldownTimer = 0.0f;
    HitFlashTimer = 0.0f;
    KnockbackVelocity = {0, 0};
    DeathTimer = 0.0f;
    PlayerWasInRange = false;
    AIState = ENEMY_IDLE;

    // Posisi disesuaikan agar hitbox center-nya tepat di pos spawn
    Position.x = pos.x - (HitboxWidth / 2.0f) - HitboxOffsetX;
    Position.y = pos.y - (HitboxHeight / 2.0f) - HitboxOffsetY;
    PatrolTarget = pos;

    PlayAnimation(Anim, IDLE, DOWN, *Def->animSet);
    Anim.position = Position;
}

/*==============================================================================
 * Enemy — Update
 *==============================================================================*/

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

        DeathTimer += Time::DELTA_TIME;
        if (DeathTimer >= DeathDuration)
            IsActive = false;

        Anim.position = Position;
        UpdateAnimation(Anim, Time::DELTA_TIME);
        return;
    }

    if (HitFlashTimer > 0)
        HitFlashTimer -= Time::DELTA_TIME;
    if (AttackCooldownTimer > 0)
        AttackCooldownTimer -= Time::DELTA_TIME;

    if (Vector2Length(KnockbackVelocity) > 0.1f)
    {
        Vector2 move = Vector2Scale(KnockbackVelocity, Time::DELTA_TIME * 60.0f);
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

    // buat ngatur sejauh apa ai enemy bisa ke update
    const float AI_UPDATE_RANGE = TILE_SIZE * 20.0f;

    if (Vector2Distance(Position, PlayerInstance.GetPosition()) <= AI_UPDATE_RANGE)
        UpdateAI();

    Anim.position = Position;
    UpdateAnimation(Anim, Time::DELTA_TIME);
}

/*==============================================================================
 * Enemy — AI
 *==============================================================================*/

void Enemy::UpdateAI()
{
    // Jika player mati, paksa idle agar enemy tidak terus mengejar posisi terakhir
    if (!PlayerInstance.IsAlive())
    {
        if (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK)
        {
            AIState = ENEMY_IDLE;
            PlayAnimation(Anim, IDLE, Anim.direction, *AnimSet);
        }
        return;
    }

    // Detection range diperluas saat mengejar agar enemy tidak langsung berhenti di tepi range
    DetectionRange = (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK)
                         ? Def->stats.chaseDetectionRange
                         : Def->stats.baseDetectionRange;

    // Regen HP hanya saat tidak agresif
    if (HealthRegenTimer > 0)
    {
        HealthRegenTimer -= Time::DELTA_TIME;
    }
    else if (AIState != ENEMY_CHASE && AIState != ENEMY_ATTACK && Health < MaxHealth)
    {
        Health += Def->stats.healthRegenRate * Time::DELTA_TIME;
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

/**
 * @brief Cek apakah player dalam jangkauan dan tidak terhalang obstacle.
 * @return True jika player terlihat
 * @note Raycast dilakukan terhadap collision layer Tiled + dynamic obstacles
 */
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
            obstacles.push_back(obj);
    }

    // Dynamic obstacles (misal: chest, benda interaktif) ikut dipertimbangkan
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

    PatrolTimer += Time::DELTA_TIME;
    if (PatrolTimer >= PatrolWaitTime)
    {
        PatrolTimer = 0;
        PatrolTarget = SpawnPoint; // fallback jika tidak ada posisi valid

        // Coba hingga 10 kali agar patrol target tidak di dalam dinding
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
    SteeringContext ctx = BuildSteeringContext();

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

    if (Steering.SteeringFlipCount >= Steering.MaxSteeringFlipCount)
    {
        Steering.SteeringFlipCount = 0;
        AIState = ENEMY_PATROL;
        return;
    }

    if (Steering.IsPlayerInRange(ctx, Ray))
    {
        MoveTowards(playerCenter, Def->stats.chaseSpeed);
    }
    else
    {
        Vector2 steerDir = Steering.Compute(STEERING_CHASE, ctx, Ray);
        Velocity = steerDir;

        if (Vector2LengthSqr(steerDir) > 0.001f)
            MoveTowards(Steering.SteeringTarget, Def->stats.chaseSpeed);
        else
            MoveTowards(playerCenter, Def->stats.chaseSpeed);
    }

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

    if (Vector2Distance(GetCenter(), SpawnPoint) < TILE_SIZE * 3.0f)
    {
        AIState = ENEMY_IDLE;
        PlayAnimation(Anim, IDLE, Anim.direction, *Def->animSet);
        return;
    }

    // throttle scan spawn flow field
    ReturnScanTimer -= Time::DELTA_TIME;
    if (ReturnScanTimer <= 0.f)
    {
        ReturnScanTimer = RETURN_SCAN_INTERVAL;

        if (ReturnFlowField == nullptr ||
            Vector2LengthSqr(ReturnFlowField->GetDirection(GetCenter())) < 0.001f)
        {
            ReturnFlowField = FindNearestSpawnFlowField(GetCenter());
        }
    }

    SteeringContext ctx = BuildSteeringContext();

    if (Steering.SteeringFlipCount >= Steering.MaxSteeringFlipCount)
    {
        Steering.SteeringFlipCount = 0;
        AIState = ENEMY_IDLE;
        return;
    }

    if (ReturnFlowField == nullptr)
    {
        // fallback — jalan lurus ke spawn point
        MoveTowards(SpawnPoint, Def->stats.speed);
        return;
    }

    Vector2 steerDir = Steering.Compute(STEERING_RETURN, ctx, Ray);
    Velocity = steerDir;

    if (Vector2LengthSqr(steerDir) > 0.001f)
        MoveTowards(Steering.SteeringTarget, Def->stats.speed);
    else
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
        // Sedikit buffer (1.2x) agar enemy tidak langsung keluar ATTACK state saat player mundur tipis
        if (dist > Def->stats.attackRange * 1.2f)
        {
            AIState = ENEMY_CHASE;
            PlayAnimation(Anim, WALK, Anim.direction, *AnimSet);
        }
    }
}

/*==============================================================================
 * Enemy — Combat
 *==============================================================================*/

void Enemy::PerformAttack()
{
    Vector2 enemyCenter = GetCenter();
    Vector2 playerCenter = PlayerInstance.GetCenter();

    // cek ada obstacle nggak antara enemy dan player
    Vector2 dir = Vector2Normalize(Vector2Subtract(playerCenter, enemyCenter));
    float dist = Vector2Distance(enemyCenter, playerCenter);
    auto obstacles = cachedObstacleList;
    RayHitResult hit = Ray.Cast(enemyCenter, dir, dist, obstacles);

    if (hit.hit)
        return; // ada obstacle, batal serang

    Vector2 knockDir = dir;
    PlayerInstance.TakeDamage(Def->stats.damage, knockDir);

    PlayAnimation(Anim, ATTACK, Anim.direction, *AnimSet);
    Anim.isAttacking = true;
    AttackCooldownTimer = AttackCooldown;
}

/**
 * @brief Enemy menerima damage dan knockback.
 * @param amount Jumlah damage
 * @param knockback Arah knockback (normalized)
 * @note HealthRegenTimer di-reset agar regen tidak langsung jalan setelah kena hit
 */
void Enemy::TakeDamage(float amount, Vector2 knockback)
{
    Entity::TakeDamage(amount, knockback);
    HitFlashTimer = 0.15f;
    KnockbackVelocity = Vector2Scale(knockback, 6.0f);
    HealthRegenTimer = Def->stats.healthRegenDelay;
}

/*==============================================================================
 * Enemy — Render
 *==============================================================================*/

void Enemy::Render()
{
    if (!IsActive)
        return;

    // Shadow sederhana di bawah enemy
    DrawEllipse((int)Position.x + 16, (int)Position.y + 30, 10, 4, {0, 0, 0, 80});

    bool shouldDraw = true;
    if (Health <= 0)
    {
        // Blink makin cepat menjelang akhir death timer
        float blinkFreq = (DeathTimer / DeathDuration) * 15.0f;
        shouldDraw = AnimEffects::ShouldBlink(DeathTimer, blinkFreq);
    }

    if (shouldDraw)
    {
        Color tint = (HitFlashTimer > 0) ? RED : WHITE;
        DrawAnimation(Anim, TEXTURE_ENEMIES, tint);
    }

    // Health bar hanya tampil saat agresif
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
        DrawRectangleLinesEx(GetHitbox(), 1.0f, VIOLET);

        if (AIState == ENEMY_CHASE || AIState == ENEMY_ATTACK)
            DrawLineEx(enemyCenter, PlayerInstance.GetCenter(), 1.0f, RED);

        // Steering debug
        if (AIState == ENEMY_CHASE || AIState == ENEMY_RETURN)
            Debug::DrawSteeringOverlay(*this);
    }
}

/*==============================================================================
 * Global Utility Functions (definisi: src/enemy.cpp)
 *==============================================================================*/

void InitEnemy()
{
    LoadTileTexture(TEXTURE_ENEMIES, "assets/textures/enemies.png");
    enemyData.Load("assets/data/enemies.json");
}

/**
 * @brief Spawn sejumlah enemy acak sekaligus.
 * @note Jumlah enemy per wave: 4–7
 */
void SpawnRandomWave()
{
    int count = GetRandomValue(4, 7);
    for (int i = 0; i < count; i++)
        SpawnRandomEnemy();
}

/**
 * @brief Spawn satu enemy acak di posisi valid dalam map.
 * @note Mencoba hingga 100 posisi acak — jika semuanya tidak valid, enemy tidak di-spawn
 */
void SpawnRandomEnemy()
{
    if (!tilesonMap)
        return;

    const auto &names = enemyData.GetAllNames();
    const std::string &picked = names[GetRandomValue(0, (int)names.size() - 1)];
    const EnemyDefinition &def = enemyData.Get(picked);

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
        Enemy *en = new Enemy();
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

/*==============================================================================
 * Enemy — Helper
 *==============================================================================*/

// Gerak per axis secara terpisah agar enemy bisa slide di sepanjang dinding
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

// Fallback ke SlimeAnimationSet jika nama tidak dikenali
const AnimationSet *ResolveAnimSet(const std::string &name)
{
    if (name == "Skeleton")
        return &SkeletonAnimationSet;
    if (name == "Wolf")
        return &WolfAnimationSet;
    return &SlimeAnimationSet;
}