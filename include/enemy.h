#pragma once
#include <cmath>
#include "../lib/raylib/include/raylib.h"
#include "animation.h"
#include "map.h"
#include "screen.h"
#include <string>

enum EnemyType{
    SLIME,
    SKELETON,
    WOLF
};

//enum EnemyState {
//    IDLE,
//    ROAMING,
//    CHASE,
//    ATTACK,
//    DEAD
//};


struct Enemy{
    Vector2 position;
    EnemyType type;


    int enemyHP;//untuk HP musuh
    int currentHP;

    int minDamage;//Minimal damage yang bisa musuh berikan
    int maxDamage;//Maksimal damage yang bisa musuh berikan

    float speed;//Kecepatan musuh
    
    float detectionRange;

    float knockbackTime;

    bool isAlive;

    AnimationEnemy EnAnim;

    float stateTime;

    Vector2 targetPos;
};

//Declaration for RNG
int GetRandomDamage(int min, int max);

void UpdateEnemy(GameState* state);

void InitEnemy();

void InitEnemyTextures();

void UpdateEnemyAI(Enemy& en, Vector2 playerPos);

Enemy CreateEnemy(Vector2 pos, EnemyType type);

extern void UpdateEnemyAI(Enemy& en, Vector2 playerPos);

void UpdateAllEnemies();

void DrawEnemyHealthBar(Enemy en);

void TakeDamage(Enemy& en, int damage);

void SpawnRandomWave();

void SpawnRandomEnemy();

void SaveEnemiesForMap(const std::string& mapPath);

bool LoadEnemiesForMap(const std::string& mapPath);

void RenderAllEnemies();

void RenderEnemy(Enemy &en);

void EnemyAttackPlayer(GameState *state);

void ClearEnemies();

extern std::vector<Enemy> activeEnemies;

extern Enemy EnemyInstance;
extern void InitEnemy();
extern void SpawnRandomEnemy();