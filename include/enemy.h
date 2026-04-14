#pragma once
#include <cmath>
#include "../lib/raylib/include/raylib.h"
#include "map.h"
#include "screen.h"

enum EnemyType{
    Slime,
    Skeleton,
    Wolf
};

enum EnemyState {
    IDLE,
    ROAMING,
    CHASE,
    ATTACK,
    DEAD
};


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

    EnemyState state;
    float stateTime;
};

//Declaration for RNG
int GetRandomDamage(int min, int max);

void UpdateEnemy(GameState* state);

void RenderEnemy(Enemy &en);

Enemy EnemyStat(Vector2 pos, int type);

void EnemyRenderTest();

void InitTextures();

void EnemyInitTest();

void EnemyAttackPlayer(GameState *state);

extern Enemy EnemyInstance;
extern void EnemyRenderTest();