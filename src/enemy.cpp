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
std::vector<Enemy> activeEnemies;

//Fungsi RNG untuk damage enemy
int GetRandomDamage(int min, int max){
    return GetRandomValue(min, max);
}

//Inisialisasi Texture Enemies
void InitEnemyTextures()
{
    LoadTileTexture(TEXTURE_ENEMIES, "texture/Enemies.png");
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
            newEnemy.speed = 75.0f;
            newEnemy.detectionRange = 175.0f;
            break;

        case SKELETON:
            newEnemy.enemyHP = 70;
            newEnemy.currentHP = 70;
            newEnemy.minDamage = 5;
            newEnemy.maxDamage = 15;
            newEnemy.speed = 100.0f;
            newEnemy.detectionRange = 200.0f;
            break;

        case WOLF:
            newEnemy.enemyHP = 100;
            newEnemy.currentHP = 100;
            newEnemy.minDamage = 5;
            newEnemy.maxDamage = 20;
            newEnemy.speed = 120.0f;
            newEnemy.detectionRange = 300.0f;
            break;
    }

    newEnemy.EnAnim.EnState = EnROAM;
    newEnemy.stateTime = GetRandomValue(1,3);

    newEnemy.knockbackTime = 0;

    const char* typeName = (type == SLIME) ? "SLIME" : (type == SKELETON) ? "SKELETON" : "WOLF";
    
    TraceLog(LOG_INFO, "ENEMY: Spawned [%s] at (%.1f, %.1f) - HP: %d", 
             typeName, pos.x, pos.y, newEnemy.enemyHP);

    return newEnemy;
}

void UpdateEnemyAI(Enemy& en, Vector2 playerPos) {
    float distToPlayer = Vector2Distance(en.position, playerPos);

    // 1. Logika Transisi State
    if (distToPlayer < en.detectionRange) {
        en.EnAnim.EnState = EnCHASE;
    } else if (en.EnAnim.EnState == EnCHASE && distToPlayer > en.detectionRange * 1.5f) {
        en.EnAnim.EnState = EnIDLE; // Berhenti ngejar kalau player jauh banget
    }

    // 2. Eksekusi Perilaku Berdasarkan State
    switch (en.EnAnim.EnState) {
        case EnIDLE:{
            en.stateTime -= GetFrameTime();
            if (en.stateTime <= 0) {
                en.EnAnim.EnState = EnROAM;
                // Cari titik random di deket dia buat didatengin
                en.targetPos = { en.position.x + GetRandomValue(-100, 100), 
                                 en.position.y + GetRandomValue(-100, 100) };
                en.stateTime = (float)GetRandomValue(2, 5);
            }
            break;
        }
        case EnROAM:{
            // Jalan pelan ke targetPos
            Vector2 moveDir = Vector2Normalize(Vector2Subtract(en.targetPos, en.position));
            en.position = Vector2Add(en.position, Vector2Scale(moveDir, en.speed * 0.5f * GetFrameTime()));
            
            if (Vector2Distance(en.position, en.targetPos) < 5.0f) en.EnAnim.EnState = EnIDLE;
            break;
        }
        case EnCHASE:{
            // Kejar player dengan speed penuh
            Vector2 chaseDir = Vector2Normalize(Vector2Subtract(playerPos, en.position));
            en.position = Vector2Add(en.position, Vector2Scale(chaseDir, en.speed * GetFrameTime()));
            break;
        }
    }
}

void UpdateAllEnemies() {
    for (auto& en : activeEnemies) {
        if (en.isAlive) {
            UpdateEnemyAI(en, PlayerInstance.GetPosition());
        }
    }
}

void DrawEnemyHealthBar(Enemy en) {
    float barWidth = 40.0f;
    float barHeight = 5.0f;
    float healthPercentage = (float)en.currentHP / (float)en.enemyHP;

    // Background Bar (Hitam/Merah Tua)
    DrawRectangle(en.position.x - barWidth/2, en.position.y + 40, barWidth, barHeight, BLACK);
    
    // Foreground Bar (Merah Terang)
    DrawRectangle(en.position.x - barWidth/2, en.position.y + 40, barWidth * healthPercentage, barHeight, RED);
}

void TakeDamage(Enemy& en, int damage) {
    en.currentHP -= damage;
    if (en.currentHP < 0) en.currentHP = 0;
    
    TraceLog(LOG_INFO, "ENEMY: Took %d damage. Remaining HP: %d", damage, en.currentHP);
    
    if (en.currentHP <= 0) {
        // Logic musuh mati (misal isAlive = false)
        TraceLog(LOG_WARNING, "ENEMY: Has been defeated!");
    }
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

    //Mengkonversi angka 1-3 menjadi EnemyType
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
    TextureAsset tex = TEXTURE_ENEMIES;
    TileType tileID;

    switch (en.type){
        case SLIME:
            tileID = TILE_ENEMY_SLIME;
            break;
        case SKELETON:
            tileID = TILE_ENEMY_SKELETON;
            break;
        case WOLF:
            tileID = TILE_ENEMY_WOLF;
            break;
        default:
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

