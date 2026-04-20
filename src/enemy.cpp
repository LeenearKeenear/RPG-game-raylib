#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "../include/animation.h"
#include "../include/map.h"
#include "../include/screen.h"
#include "../include/entities.h"
#include "../include/player.h"
#include "../include/enemy.h"
#include "../include/mapLogic.h"
#include <vector>
#include <map>
#include <string>


static std::vector<Enemy> currentEnemies;
static std::map<std::string, std::vector<Enemy>> savedMapEnemies;

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
}

void SaveEnemiesForMap(const std::string& mapPath)
{
    if (mapPath.empty()) return;
    savedMapEnemies[mapPath] = currentEnemies;
}

bool LoadEnemiesForMap(const std::string& mapPath)
{
    if (mapPath.empty()) return false;
    
    auto it = savedMapEnemies.find(mapPath);
    if (it != savedMapEnemies.end())
    {
        currentEnemies = it->second;
        return true;
    }
    return false;
}

void ClearEnemies()
{
    currentEnemies.clear();
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
    if (tilesonMap == nullptr) return;

    Vector2 randomPos;
    bool validPos = false;
    int maxAttempts = 50;
    int attempts = 0;

    float mapWidth = (float)tilesonMap->width * TILE_SIZE;
    float mapHeight = (float)tilesonMap->height * TILE_SIZE;

    // Ambil data collision map sekali
    TiledHelper::CollisionResult mapCollision;
    TiledHelperFunction.TryGetCollisionByLayerName(COLLISION_LAYER_NAME, mapCollision);

    while (!validPos && attempts < maxAttempts)
    {
        randomPos = { 
            (float)GetRandomValue(TILE_SIZE, (int)mapWidth - TILE_SIZE), 
            (float)GetRandomValue(TILE_SIZE, (int)mapHeight - TILE_SIZE) 
        };

        // Buat hitbox sementara untuk musuh (asumsi 32x32)
        Rectangle enemyHitbox = BuildHitbox(randomPos, 0, 0, TILE_SIZE, TILE_SIZE);

        // Cek apakah posisi ini menabrak wall atau polygon collision
        bool colRect = CheckCollisionAgainstRects(enemyHitbox, mapCollision.rects);
        bool colPoly = CheckCollisionAgainstPolygons(enemyHitbox, mapCollision.polygons);
        bool inBounds = IsWithinWorldBounds(enemyHitbox, mapWidth, mapHeight);

        if (!colRect && !colPoly && inBounds)
        {
            validPos = true;
        }
        attempts++;
    }

    if (!validPos) return; // Gagal nemu tempat kosong

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
            tex = TEXTURE_SLIME;
            tileID = TILE_ENEMY_SKELETON;
            break;
        case WOLF:
            tex = TEXTURE_SLIME;
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

