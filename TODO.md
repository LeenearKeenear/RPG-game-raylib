# Tugas yang harus diselesaikan

## 📌 Aturan Penulisan Kode (Syntax)

### 1. Hindari Magic Number

Jangan langsung memasukkan angka ke dalam parameter function.

❌ Contoh:

```cpp
InitWindow(1280, 720, "Dungeon Game");
```

✅ Gunakan variabel:

```cpp
int screenWidth = 1280;
int screenHeight = 720;

InitWindow(screenWidth, screenHeight, "Dungeon Game");
```

Tujuan: agar kode lebih mudah dibaca dan gampang diubah.

### 2. Penamaan Variabel & Function

Gunakan PascalCase jika terdiri dari lebih dari satu kata.

Contoh:

```cpp
int GameScreenWidth;
void DrawTextureText();
```

### 3. Wajib Menggunakan Komentar

Setiap function harus punya deskripsi singkat.
Syntax yang kompleks wajib diberi penjelasan.

Contoh:

```cpp
// Fungsi untuk generate karakter player
void DrawCharacter(){}
```

### 4. Jangan Terlalu Sering Pull Request

Kumpulkan perubahan secukupnya sebelum membuat PR agar repo tetap rapi.

### 5. Struktur File (.cpp)

Semua file .cpp harus diletakkan di folder: src/

### 6. Struktur File Header (.h)

Semua file .h harus diletakkan di folder: include/

## 📌 To do

Usahakan diurutkan berdasarkan prioritas; dari yang paling penting ke kurang penting

- [ ] Buat Data Struct buat item, player, enemy sama data data struct yang penting
- [ ] Desain Layout menu secara kasar
- [ ] Implementasi pertama tile map sama player movement
- [ ] Migrasi projek menggunakan `Cmake`

#### Todo Multi-Map Preparation
Diurutkan berdasarkan prioritas

#### Fondasi
- [ ] Bikin `struct MapData` yang nyimpen `width`, `height`, array tile, dan spawn point
- [ ] Ganti `WorldMap[][]` dari hardcode jadi dinamis ikut ukuran `MapData`
- [ ] Pindahin `WORLD_WIDTH` dan `WORLD_HEIGHT` ke dalam `struct MapData`
- [ ] Bikin variabel global `CurrentMap` (pointer ke `MapData` yang lagi aktif)

#### Load & Unload
- [ ] Bikin fungsi `LoadMap(const char* file)` buat load data map dari file
- [ ] Bikin fungsi `UnloadMap()` buat cleanup memori map sebelum swap

#### Refactor Fungsi yang Bergantung ke `WORLD_WIDTH`/`WORLD_HEIGHT`
- [ ] `PlayerMovement()` — `MapBounds` harus ngambil dari `CurrentMap`
- [ ] `PlayerCamera()` — `mapW` dan `mapH` harus dari `CurrentMap`
- [ ] `RenderMap()` — loop render harus ikut ukuran `CurrentMap`
- [ ] `DebugMenu()` — `MapBounds` harus dari `CurrentMap`

#### Inisialisasi
- [ ] `InitAll()` — spawn point player ngambil dari data `CurrentMap`, bukan hardcode
- [ ] `InitAll()` — posisi kamera awal ikut spawn point `CurrentMap`

#### Object Layer
- [ ] `Door` dan object lain dibaca dari object layer `CurrentMap`, bukan hardcode