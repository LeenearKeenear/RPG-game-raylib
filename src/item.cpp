#include "../include/item.h"
#include "../include/animation.h"
#include "../include/screen.h"
#include "../include/entities.h"
#include "../include/player.h"
#include "../include/enemy.h"
#include "../include/mapLogic.h"
#include <iostream>

// Inisialisasi list item kosong
std::vector<Item> activeItems;

void LoadItemTexture(){
    LoadTileTexture(TEXTURE_ITEMS, "texture/map.png");
}

void SpawnItem(ItemCategory category, Vector2 pos, float multiplier, ItemRarity rarity) {
    Item newItem;
    newItem.category = category;
    newItem.position = pos;
    newItem.statMultiplier = multiplier;
    newItem.rarity = rarity;
    newItem.isPickedUp = false;

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

    // Tentukan Nama dan Hitbox berdasarkan kategori (Requirement 2 & 3)
    switch (category) {
        case ITEM_WEAPON:
            newItem.name = "Sword";
            newItem.hitbox = {pos.x, pos.y, 32, 32}; // Hitbox default 32x32
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

    TraceLog(LOG_INFO, "ITEM: Spawned '%s' at (%.1f, %.1f) with Multiplier: %.2f", 
     newItem.name.c_str(), pos.x, pos.y, multiplier);

    // Masukkan ke dunia (Vector)
    activeItems.push_back(newItem);
}

// Wrapper spesifik buat Weapon (Tinggal panggil SpawnItem)
void SpawnWeapon(Vector2 pos, float multiplier, ItemRarity rarity) {
    SpawnItem(ITEM_WEAPON, pos, multiplier, rarity);
}

void SpawnPotion(Vector2 pos, float multiplier, ItemRarity rarity) {
    SpawnItem(ITEM_POTION, pos, multiplier, rarity);
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

void RenderItems() {

    TextureAsset tex = TEXTURE_ITEMS;
    TileType tileID;

    for (const auto& item : activeItems) {
        if (!item.isPickedUp) {
            // Gambar kotak sementara sebagai pengganti texture
            //DrawRectangleRec(item.hitbox, BLUE); 
            //DrawText(item.name.c_str(), item.position.x, item.position.y - 10, 10, WHITE);
            switch (item.category)
            {
            case ITEM_POTION:
                tileID = TILE_ITEM_POTION;
                break;
            default:
                break;
            }

    
        }
        RenderTilePNG(item.position.x, item.position.y, tileID, 0.0f, tex);
    }
    
}