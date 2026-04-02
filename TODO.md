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

## 📌 To do :

1. 
2.
