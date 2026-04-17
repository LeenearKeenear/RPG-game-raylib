#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/screen.h"
#include "../include/Map.h"
#include "../include/entities.h"
#include "../include/player.h"
#include "../include/enemy.h"
#include "../include/animation.h"

//EnemyTest EnemyInstance;

static Enemy entest;
//Fungsi RNG untuk damage enemy
int GetRandomDamage(int min, int max){
    return GetRandomValue(min, max);
}

//void InitTextures()
//{
//    LoadTileTexture(TEXTURE_SLIME, "texture/Enemies.png");
//    LoadTileTexture(TEXTURE_SKELETON, "texture/Enemies.png");
//    LoadTileTexture(TEXTURE_WOLF, "texture/Enemies.png");
//}

//Fungsi untuk membuat enemy dengan stat yang berbeda
Enemy EnemyStat(Vector2 pos, int type)
{
    entest.position = pos;
    entest.isAlive = true;


    switch (type){
        case 0://Slime
            entest.enemyHP = 40;
            entest.currentHP = 40;
            entest.minDamage = 2;
            entest.maxDamage = 10;
            entest.speed = 50.0f;
            entest.detectionRange = 175.0f;
            break;

        case 1://Skeleton
            entest.enemyHP = 70;
            entest.currentHP = 70;
            entest.minDamage = 5;
            entest.maxDamage = 15;
            entest.speed = 75.0f;
            entest.detectionRange = 200.0f;
            break;

        case 2://Wolf
            entest.enemyHP = 100;
            entest.currentHP = 100;
            entest.minDamage = 5;
            entest.maxDamage = 20;
            entest.speed = 100.0f;
            entest.detectionRange = 300.0f;
            break;
    }

    EnAnim State = IDLE;
    entest.stateTime = GetRandomValue(1,3);

    entest.knockbackTime = 0;

    return entest;
}

//Testing enemy Init
void EnemyInitTest()
{
    // load texture (sekali)
    LoadTileTexture(TEXTURE_SLIME, "texture/Enemies.png");

    // create enemy
    entest.position = {6, 5};
    entest.isAlive = true;
    entest.type = Slime;
}

//Testing render enemy
void EnemyRenderTest()
{
    if (!entest.isAlive) return;

    RenderTilePNG(
        entest.position.x * 32,
        entest.position.y * 32,
        TILE_ENEMY_SLIME,
        0.0f,
        TEXTURE_SLIME
    );
}

void RenderEnemy(Enemy &en)
{
    if (!entest.isAlive) return;
    TextureAsset tex;

   //switch (entest.type)
    //{
    //    case 0: tex = TEXTURE_SLIME; break;
    //    case 1: tex = TEXTURE_SKELETON; break;
    //    case 2: tex = TEXTURE_WOLF; break;
    //    default: tex = TEXTURE_SLIME; break;
    //}

    RenderTilePNG(
        entest.position.x,
        entest.position.y,
        TILE_ENEMY_SLIME,
        0.0f,
        TEXTURE_SLIME
    );
}

