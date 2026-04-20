#include "../include/item.h"
#include <iostream>

// Inisialisasi list item kosong
std::vector<Item> activeItems;

void SpawnItem(ItemCategory category, Vector2 pos, float multiplier, ItemRarity rarity) {
    Item newItem;
    newItem.category = category;
    newItem.position = pos;
    newItem.statMultiplier = multiplier;
    newItem.rarity = rarity;
    newItem.isPickedUp = false;

    // Tentukan Nama dan Hitbox berdasarkan kategori (Requirement 2 & 3)
    switch (category) {
        case ITEM_WEAPON:
            newItem.name = "Rusty Sword";
            newItem.hitbox = {pos.x, pos.y, 32, 32}; // Hitbox default 32x32
            break;
        case ITEM_POTION:
            newItem.name = "Health Potion";
            newItem.hitbox = {pos.x, pos.y, 20, 20}; 
            break;
        case ITEM_POISON:
            newItem.name = "Deadly Poison";
            newItem.hitbox = {pos.x, pos.y, 20, 20};
            break;
        default:
            newItem.name = "Unknown Item";
            newItem.hitbox = {pos.x, pos.y, 16, 16};
            break;
    }

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
    for (const auto& item : activeItems) {
        if (!item.isPickedUp) {
            // Gambar kotak sementara sebagai pengganti texture
            DrawRectangleRec(item.hitbox, BLUE); 
            DrawText(item.name.c_str(), item.position.x, item.position.y - 10, 10, WHITE);
        }
    }
}