#include "../include/item.h"
#include "../include/animation.h"
#include "../include/screen.h"
#include "../include/entities.h"
#include "../include/player.h"
#include "../include/enemy.h"
#include "../include/mapLogic.h"
#include <iostream>
#include <vector>

static  std::vector<Item> currentItems;
static std::map<std::string, std::vector<Item>> savedMapItems;
std::vector<Item> activeItems;
extern void DrawSmallSprite(TextureAsset slot, Vector2 sheetCoord, Vector2 worldPos, float scale);
extern TileDefinition TileProperty[];

void InitItemTextures(){
    LoadTileTexture(TEXTURE_ITEMS, "texture/map.png");
}

void SpawnItemWave(){
    int jumlahItem = GetRandomValue(1,4);

    for (int i = 0; i<jumlahItem; i++){
        SpawnRandomItem();
    }
}

void InitItems(){
    InitItemTextures();
    SpawnItemWave();
}

void SaveItemsForMap(const std::string& mapPath){
    if (mapPath.empty()) return;
    savedMapItems[mapPath] = currentItems;
}

bool LoadItemsforMap(const std::string& mapPath){
    if (mapPath.empty()) return false;

    auto it = savedMapItems.find(mapPath);
    if (it != savedMapItems.end()){
        currentItems = it->second;
        return true;
    }
    return false;
}

void ClearItems(){
    currentItems.clear();
}

void SpawnRandomItem(){
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
        Rectangle itemHitbox = BuildHitbox(randomPos, 0, 0, TILE_SIZE, TILE_SIZE);

        // Cek apakah posisi ini menabrak wall atau polygon collision
        bool colRect = CheckCollisionAgainstRects(itemHitbox, mapCollision.rects);
        bool colPoly = CheckCollisionAgainstPolygons(itemHitbox, mapCollision.polygons);
        bool inBounds = IsWithinWorldBounds(itemHitbox, mapWidth, mapHeight);

        if (!colRect && !colPoly && inBounds)
        {
            validPos = true;
        }
        attempts++;
    }

    if (!validPos) return; // Gagal nemu tempat kosong

    int randomItem = GetRandomValue(1,2);
    int randomMult = GetRandomValue(1,3);
    int randomRarity = GetRandomValue(1,2);

    ItemCategory c;
    if (randomItem == 1) c = ITEM_POTION;
    else c = ITEM_WEAPON;

    ItemRarity r;
    if(randomRarity == 1) r = RARITY_COMMON;
    else r = RARITY_RARE;

    currentItems.push_back(SpawnItem(randomPos, c, randomMult, r));
}

Item SpawnItem(Vector2 pos, ItemCategory category, float multiplier, ItemRarity rarity) {
    Item newItem;
    newItem.category = category;
    newItem.statMultiplier = multiplier;
    newItem.rarity = rarity;
    newItem.isPickedUp = false;
    newItem.position = pos;
    // Tentukan Nama dan Hitbox berdasarkan kategori (Requirement 2 & 3)
    switch (category) {
        case ITEM_WEAPON:
            newItem.name = "Sword";
            newItem.hitbox = {pos.x, pos.y, 32, 32};
            break;
        case ITEM_POTION:
            newItem.name = "Health Potion";
            newItem.hitbox = {pos.x, pos.y, 20, 20}; 
            break;
        default:
            newItem.name = "Unknown Item";
            newItem.hitbox = {pos.x, pos.y, 16, 16};
            break;
    }

    // Tentukan nama rarity-nya dulu
    const char* rarityText = "";
    if (rarity == RARITY_COMMON) rarityText = "COMMON";
    else if (rarity == RARITY_RARE) rarityText = "RARE";

    TraceLog(LOG_INFO, "ITEM: Spawned '%s' at (%.1f, %.1f) with Multiplier: %.2f and rarity '%s'", 
         newItem.name.c_str(), pos.x, pos.y, multiplier, rarityText);
    // Masukkan ke dunia (Vector)
    return newItem;
}

void UpdateItems(Rectangle playerHitbox) {
    for (size_t i = 0; i < activeItems.size(); i++) {
        if (!activeItems[i].isPickedUp) {
            // Cek collision antara player dan hitbox item
            if (CheckCollisionRecs(playerHitbox, activeItems[i].hitbox)) {
                activeItems[i].isPickedUp = true;
                std::cout << "Player picked up: " << activeItems[i].name << std::endl;
                // Di sini nanti bisa ditambah logic masukin ke inventory
            }
        }
    }
}

void RenderAllItems(){
    for(auto& Item : currentItems){
        if (!Item.isPickedUp){
            RenderItems(Item);
        }
    }
}

void RenderItems(Item& item) {
    // Tentukan koordinat di spritesheet secara manual saja kalau dilarang akses TileProperty
    Vector2 sheetCoord;

    switch(item.category){
        case ITEM_POTION:
            sheetCoord = {7, 8};
            break;
        case ITEM_WEAPON:
            sheetCoord = {6, 4};
            break;
        default:
            sheetCoord = {7, 8};
            break;
    }

    // Panggil fungsi pembungkus tadi
    DrawSmallSprite(TEXTURE_ITEMS, sheetCoord, item.position, 0.5f);
}