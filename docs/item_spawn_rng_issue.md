# Issue: Item Spawn RNG tidak Deterministic

> **Human:** "nah nemu masalah lagi. sistem capture buat restart nya gak nge capture 100% soalnya gini bisa kamu cek di sistem items terutama spawn di map. dia itu pipeline nya bakal milih semua object layer item lalu pilih mana yang bakal aktif buat nge spawn item. nah karena random, jadi sistem capture nya gak bener bener capture posisi item yang di spawn di world."
>
> **AI:** Oh, maksudnya item spawn pake RNG. Cache nyimpen hasil akhir, tapi pas fallback `SpawnItemWave()` spawn ulang fresh dengan RNG beda.
>
> **Human:** "berlaku juga buat sistem save datanya gak? itu bug"
>
> **AI:** Iya, berlaku juga. Loading screen fallback `SpawnItemWave()` kalo save gak ada → spawn random.

## Masalah

`SpawnItemWave()` memanggil:

1. `spawnManager.DetermineActiveAreas()` — milih area spawn mana yang aktif secara random
2. `spawnManager.SpawnAll()` — spawn item dengan definisi & posisi random

Keduanya pake RNG murni (`GetRandomValue()` / `std::mt19937`) **tanpa seed deterministic**.  
Akibatnya, tiap kali spawn fresh:
- Posisi item bisa berbeda
- Jenis item bisa berbeda
- Jumlah item bisa berbeda

## Affected Flows (Semua Tipe Map)

| Flow | Map Type | Trigger |
|---|---|---|
| Loading screen map switch | Worldgen + Non-worldgen | `LoadItemsForMapDir()` gagal → `SpawnItemWave()` |
| Pause menu restart (cache fallback) | Worldgen + Non-worldgen | `.cache` gak ada → `SpawnItemWave()` |
| First time masuk map (no save) | Non-worldgen | Save file belum ada → fallback spawn |
| Worldgen `InitAll()` / `RunWorldgen()` | Worldgen | Panggil `InitItems()` → `SpawnItemWave()` |

## Root Cause

- `ItemSpawnManager::DetermineActiveAreas()` — `ItemSpawnManager::CategorizeAreas()` — semua pake `GetRandomValue()` tanpa seed
- `ItemSpawnManager::SpawnAll()` — `SpawnRandomItemAtArea()` — pake `std::mt19937` dengan `time(nullptr)` sebagai fallback seed
- `SpawnItemWave()` di `item.cpp` — clear + re-init + spawn ulang tanpa deterministic seed

```cpp
// src/items/item.cpp
void SpawnItemWave()
{
    itemData.activeItems.clear();
    spawnManager.Init(ITEM_LAYER_NAME);   // baca object layer dari map
    spawnManager.SpawnAll(itemData.activeItems); // RNG tanpa seed tetap
}
```

## Lokasi Code

| File | Baris | Fungsi |
|---|---|---|
| `include/items/item.h` | 314-356 | `ItemSpawnManager`, `DetermineActiveAreas()`, `SpawnAll()` |
| `src/items/item.cpp` | 68-73 | `SpawnItemWave()` |
| `src/items/item.cpp` | 588+ | `DetermineActiveAreas()` — RNG area aktif |
| `src/items/item.cpp` | 620+ | `SpawnAll()` — RNG spawn item |
| `src/core/loading_screen.cpp` | 163-165 | Fallback `SpawnItemWave()` di map switch |
| `src/core/screen_handler.cpp` | 93-94 | `InitItems()` di `InitAll()` |
| `src/ui/pauseMenu.cpp` | 422-423 | Fallback `SpawnItemWave()` di restart |

## Rekomendasi Fix

### Opsi 1: Deterministic Seed
Inject seed dari system yang deterministic:
- **Worldgen maps:** pake `g_SeedManager.GetSeed(stageIdx)` sebagai RNG seed
- **Non-worldgen maps:** pake hash dari `mapPath` sebagai RNG seed

### Opsi 2: Auto-Capture
Auto-save item state ke `.cache` sehabis spawn pertama.  
Jangan pernah spawn ulang fresh kalo cache/save ada — load dari disk aja.

### Opsi 3: Gabungan
Pake deterministic seed buat spawn awal + auto-capture sebagai fallback.
