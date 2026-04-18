#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/animation.h"
#include "../include/map.h"
#include "../include/screen.h"
#include "../include/entities.h"
#include "../include/player.h"
#include "../include/enemy.h"
#include <vector>


static std::vector<Enemy> currentEnemies;
static Enemy entest;
//Fungsi RNG untuk damage enemy
int GetRandomDamage(int min, int max){
    return GetRandomValue(min, max);
}

//Inisialisasi Texture Enemies
void InitEnemyTextures()
{
    LoadTileTexture(TEXTURE_SLIME, "texture/Enemies.png");
    LoadTileTexture(TEXTURE_SKELETON, "texture/Enemies.png");
    LoadTileTexture(TEXTURE_WOLF, "texture/Enemies.png");
}

void InitEnemy(){
    InitEnemyTextures();
    SpawnRandomWave();
}

//Fungsi untuk membuat enemy dengan stat yang berbeda
Enemy CreateEnemy(Vector2 pos, EnemyType type)
{   
    Enemy newEnemy;

    newEnemy.position = pos;
    newEnemy.isAlive = true;
    newEnemy.type = type;


    switch (type){
        case SLIME:
            newEnemy.enemyHP = 40;
            newEnemy.currentHP = 40;
            newEnemy.minDamage = 2;
            newEnemy.maxDamage = 10;
            newEnemy.speed = 50.0f;
            newEnemy.detectionRange = 175.0f;
            break;

        case SKELETON:
            newEnemy.enemyHP = 70;
            newEnemy.currentHP = 70;
            newEnemy.minDamage = 5;
            newEnemy.maxDamage = 15;
            newEnemy.speed = 75.0f;
            newEnemy.detectionRange = 200.0f;
            break;

        case WOLF:
            newEnemy.enemyHP = 100;
            newEnemy.currentHP = 100;
            newEnemy.minDamage = 5;
            newEnemy.maxDamage = 20;
            newEnemy.speed = 100.0f;
            newEnemy.detectionRange = 300.0f;
            break;
    }

    //newEnemy.EnAnim.EnState = EnIDLE;
    newEnemy.stateTime = GetRandomValue(1,3);

    newEnemy.knockbackTime = 0;

    return newEnemy;
}

void SpawnRandomWave(){
    int jumlahEnemy = GetRandomValue(4,7);

    for(int i = 0; i< jumlahEnemy; i++){
        SpawnRandomEnemy();
    }
}

void SpawnRandomEnemy(){
    Vector2 randomPos = { (float)GetRandomValue(100, 700), (float)GetRandomValue(100, 500) };

    //Menggunakan GetRandomValue untuk spawn enemy
    int randomInt = GetRandomValue(1, 3);

    //Mengkoversi angka 1-3 menjadi EnemyType
    EnemyType t;
    if (randomInt == 1) t = SLIME;
    else if (randomInt == 2) t = SKELETON;
    else t = WOLF;

    currentEnemies.push_back(CreateEnemy(randomPos, t));
}

void RenderAllEnemies(){
    //Melakukan looping untuk semua musuh yang ada di vector
    for(auto &en : currentEnemies){
        if (en.isAlive){
            RenderEnemy(en);
        }
    }
}

//Testing enemy Init
//void EnemyInitTest()
//{
//    // load texture (sekali)
//   LoadTileTexture(TEXTURE_SLIME, "texture/Enemies.png");
//
//    // create enemy
//    newEnemy.position = {6, 5};
//    newEnemy.isAlive = true;
//    newEnemy.type = SLIME;
//}

//Testing render enemy
//void EnemyRenderTest()
//{
//    if (!newEnemy.isAlive) return;
//
//    RenderTilePNG(
//        newEnemy.position.x * 32,
//        newEnemy.position.y * 32,
//        TILE_ENEMY_SLIME,
//        0.0f,
//        TEXTURE_SLIME
//    );
//}

void RenderEnemy(Enemy &en)
{
    TextureAsset tex;
    TileType tileID;

    switch (en.type){
        case SLIME:
            tex = TEXTURE_SLIME;
            tileID = TILE_ENEMY_SLIME;
            break;
        case SKELETON:
            tex = TEXTURE_SKELETON;
            tileID = TILE_ENEMY_SKELETON;
            break;
        case WOLF:
            tex = TEXTURE_WOLF;
            tileID = TILE_ENEMY_WOLF;
            break;
        default:
            tex = TEXTURE_SLIME;
            tileID = TILE_ENEMY_SLIME;
            break;
    }

    RenderTilePNG(
        en.position.x,
        en.position.y,
        tileID,
        0.0f,
        tex
    );
}

