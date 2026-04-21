#pragma once
#include "../lib/raylib/include/raylib.h"
#include <vector>
#include <string>

// Enum untuk membedakan kategori item
typedef enum {
    ITEM_WEAPON,
    ITEM_POTION,
    ITEM_POISON,
    ITEM_ARMOR
} ItemCategory;

// Enum untuk kelangkaan (Rarity)
typedef enum {
    RARITY_COMMON,
    RARITY_RARE
} ItemRarity;

// Struktur data Item
typedef struct {
    std::string name;
    ItemCategory category;
    ItemRarity rarity;
    Vector2 position;
    Rectangle hitbox;    // Buat interaksi (diambil player)
    int textureID;       // Index texture di atlas
    bool isPickedUp;     // Status apakah sudah diambil
    float statMultiplier; // Multiplier spawn (misal: damage x2)
} Item;

// Global list untuk item yang ada di map
extern std::vector<Item> activeItems;

void LoadItemTexture();

// Fungsi utama untuk spawn
void SpawnItem(ItemCategory category, Vector2 pos, float multiplier, ItemRarity rarity);

// Fungsi pembantu untuk kategori spesifik
void SpawnWeapon(Vector2 pos, float multiplier, ItemRarity rarity);
void SpawnPotion(Vector2 pos, float multiplier, ItemRarity rarity);

// Fungsi Render & Update
void UpdateItems(Rectangle playerHitbox);
void RenderItems();

extern void RenderItems();