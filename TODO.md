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
- [x] Migrasi projek menggunakan `Cmake`

## Todo Multi-Map - Phase 2: File Format & Parsing

### Tentukan Format Data

- [ ] Pilih format level: JSON (rekomendasi) atau TMX

### Tooling

- [ ] Install/setup Tiled Map Editor

### Parser
- [ ] Refactor `LoadMap()` dari hardcode ke baca file JSON

### Object Layer

- [ ] Parse spawn point player dari JSON
- [ ] Parse posisi `Door` dari JSON
- [ ] (Opsional) Parse posisi enemy spawn dari JSON

### Multi-Map Switching

- [ ] Tentuin trigger pindah map (masuk door, area trigger, dll)
- [ ] Bikin fungsi `SwitchMap(const char* file)` yang `UnloadMap()` dulu baru `LoadMap()` baru
- [ ] Test pindah dari map 1 ke map 2
