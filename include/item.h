#pragma once
#include "../lib/raylib/include/raylib.h"
#include <vector>
#include <string>

// Enum untuk membedakan kategori item
typedef enum
{
    ITEM_WEAPON,
    ITEM_POTION,
    ITEM_POISON,
    ITEM_ARMOR,
    ITEM_NONE
} ItemCategory;

// Enum untuk kelangkaan (Rarity)
typedef enum
{
    RARITY_COMMON,
    RARITY_RARE
} ItemRarity;

// Struktur data Item
typedef struct
{
    std::string name;
    ItemCategory category;
    ItemRarity rarity;
    Vector2 position;
    Rectangle hitbox;     // Buat interaksi (diambil player)
    int textureID;        // Index texture di atlas
    bool isPickedUp;      // Status apakah sudah diambil
    float statMultiplier; // Multiplier spawn (misal: damage x2)
} Item;

// Global list untuk item yang ada di map
extern std::vector<Item> activeItems;

// Definisi: src/item.cpp
void InitItemTextures();
void InitItems();
void SpawnItemWave();
void SpawnRandomItem();
Item SpawnItem(Vector2 pos, ItemCategory category, float multiplier, ItemRarity rarity);

void SaveItemsForMap(const std::string &mapPath);
bool LoadItemsforMap(const std::string &mapPath);
void ClearItems();

// Fungsi pembantu untuk kategori spesifik
void SpawnWeapon(Vector2 pos, float multiplier, ItemRarity rarity);
void SpawnPotion(Vector2 pos, float multiplier, ItemRarity rarity);

// Fungsi Render & Update (definisi: src/item.cpp)
void UpdateItems(Vector2 playerCenter, Rectangle playerHitbox, float magnetRadius, float itemSpeed);
void RenderItems(Item &item);
void RenderAllItems();
