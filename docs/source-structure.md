# Struktur Kode Sumber

Dokumentasi ini menjelaskan struktur folder `src/` (file `.cpp`) dan `include/` (file `.h`) proyek RPG Game Raylib, hasil restrukturisasi terbaru. Proyek menggunakan sistem *unity build* di mana semua file `.cpp` digabung menjadi satu unit kompilasi.

## Pohon Struktur Kode Sumber

### Folder `src/` (File Sumber)
```
src/
├── core/                   # Logika inti game
│   ├── game_state_saver.cpp # Penyimpanan state game (save/load)
│   ├── loading_screen.cpp   # Layar loading saat memuat aset/peta
│   ├── main.cpp             # Entry point aplikasi
│   ├── mapstack.cpp         # Manajemen stack peta (navigasi antar peta)
│   └── screen_handler.cpp   # Manajemen layar (virtual screen 640x360)
├── entities/               # Entitas game (objek yang ada di dunia game)
│   ├── enemy.cpp            # Logika musuh
│   ├── entities.cpp         # Logika dasar semua entitas
│   └── player.cpp           # Logika pemain
├── items/                  # Sistem item dan inventory
│   ├── inventory.cpp        # Manajemen inventory pemain
│   ├── item.cpp             # Logika item dasar
│   └── propsbehavior.cpp    # Perilaku item/props di dunia game
├── map/                    # Logika pemetaan dan tiles
│   ├── map.cpp              # Manajemen peta utama
│   ├── mapLogic.cpp         # Logika interaksi peta
│   └── tiles.cpp            # Manajemen tile dan rendering tilemap
├── systems/                # Sistem game (logika lintas fitur)
│   ├── combat.cpp           # Logika sistem combat
│   ├── effects.cpp          # Efek visual/suara game
│   ├── input.cpp            # Manajemen input pemain
│   ├── inputLinkedList.cpp  # Struktur data linked list untuk input
│   ├── interaction.cpp      # Logika interaksi pemain dengan objek
│   └── movement.cpp         # Logika pergerakan entitas
├── rendering/              # Logika rendering gambar
│   ├── animation.cpp        # Sistem animasi sprite
│   └── hud.cpp              # Rendering Heads-Up Display (HUD)
├── ui/                     # Antarmuka pengguna (UI)
│   ├── audioTab.cpp         # Tab pengaturan audio di menu pause
│   ├── keybindsTab.cpp      # Tab pengaturan tombol di menu pause
│   ├── mainMenu.cpp         # Menu utama game
│   ├── pauseMenu.cpp        # Menu pause game
│   ├── popup.cpp            # Komponen popup pesan
│   └── videoTab.cpp         # Tab pengaturan video di menu pause
├── debug/                  # Fitur mode debug
│   └── debugmode.cpp        # Logika aktivasi dan fitur debug
└── utils/                  # Utilitas tambahan (jika ada)
    └── (effectQueue.cpp)    # Antrian efek (jika ada file sumbernya)
```

### Folder `include/` (File Header)
```
include/
├── core/                   # Header inti
│   ├── game_state_saver.h
│   ├── loading_screen.h
│   ├── mapstack.h
│   └── screen.h
├── entities/               # Header entitas
│   ├── enemy.h
│   ├── entities.h
│   ├── entity.h
│   └── player.h
├── items/                  # Header item
│   ├── inventory.h
│   ├── item.h
│   └── propsbehavior.h
├── map/                    # Header peta
│   ├── map.h
│   ├── mapLogic.h
│   └── tiles.h
├── systems/                # Header sistem
│   ├── effects.h
│   ├── input.h
│   ├── inputLinkedList.h
│   ├── interaction.h
│   └── movement.h
├── rendering/              # Header rendering
│   ├── animation.h
│   └── hud.h
├── ui/                     # Header UI
│   ├── audioTab.h
│   ├── button.h
│   ├── buttonImg.h
│   ├── buttonTxt.h
│   ├── keybindsTab.h
│   ├── mainMenu.h
│   ├── pauseMenu.h
│   ├── popup.h
│   └── videoTab.h
├── debug/                  # Header debug
│   └── debug.h
└── utils/                  # Header utilitas
    └── effectQueue.h
```

## Daftar File per Kategori

### Core (Inti Game)
| File | Tujuan |
|------|--------|
| `main.cpp` | Entry point aplikasi, inisialisasi Raylib dan game loop |
| `screen_handler.cpp` | Mengatur virtual screen 640x360 agar tetap proporsional di berbagai resolusi |
| `loading_screen.cpp` | Menampilkan layar loading saat memuat aset atau peta |
| `mapstack.cpp` | Melacak stack peta yang sedang aktif (untuk navigasi antar peta) |
| `game_state_saver.cpp` | Menyimpan dan memuat state game (save/load) |

### Entities (Entitas)
| File | Tujuan |
|------|--------|
| `player.cpp` | Logika pergerakan, animasi, dan interaksi pemain |
| `enemy.cpp` | Logika AI, pergerakan, dan serangan musuh |
| `entities.cpp` | Logika dasar semua entitas (posisi, kolisi, rendering) |

### Items (Item & Inventory)
| File | Tujuan |
|------|--------|
| `item.cpp` | Logika dasar item (nama, tipe, efek) |
| `inventory.cpp` | Manajemen inventory pemain (tambah, hapus, gunakan item) |
| `propsbehavior.cpp` | Perilaku item saat ditaruh di dunia game (properti, interaksi) |

### Map (Peta)
| File | Tujuan |
|------|--------|
| `map.cpp` | Memuat dan mengelola peta dari file JSON Tileson |
| `tiles.cpp` | Rendering tilemap dan deteksi kolisi tile |
| `mapLogic.cpp` | Logika interaksi peta (transisi antar peta, event peta) |

### Systems (Sistem Game)
| File | Tujuan |
|------|--------|
| `input.cpp` | Memproses input keyboard/pemain |
| `movement.cpp` | Logika pergerakan semua entitas |
| `combat.cpp` | Logika serangan, damage, dan HP |
| `interaction.cpp` | Logika interaksi pemain dengan objek/npc |
| `effects.cpp` | Efek visual (partikel, transisi) dan audio |
| `inputLinkedList.cpp` | Struktur data linked list untuk antrean input |

### Rendering (Rendering)
| File | Tujuan |
|------|--------|
| `animation.cpp` | Sistem animasi sprite berbasis frame |
| `hud.cpp` | Rendering HUD (HP bar, inventory, minimap) |

### UI (Antarmuka Pengguna)
| File | Tujuan |
|------|--------|
| `mainMenu.cpp` | Menu utama (mulai game, pengaturan, keluar) |
| `pauseMenu.cpp` | Menu pause (lanjutkan, pengaturan, keluar) |
| `popup.cpp` | Komponen popup untuk pesan singkat |
| `audioTab.cpp` | Pengaturan volume audio di menu pause |
| `videoTab.cpp` | Pengaturan resolusi/fullscreen di menu pause |
| `keybindsTab.cpp` | Pengaturan tombol kontrol di menu pause |

### Debug (Mode Debug)
| File | Tujuan |
|------|--------|
| `debugmode.cpp` | Fitur debug (tampilkan FPS, kolisi, teleportasi) |

### Utils (Utilitas)
| File | Tujuan |
|------|--------|
| `effectQueue.h` | Header antrian efek (digunakan oleh sistem effects) |

## Ketergantungan Antar Modul

Berikut adalah ketergantungan utama antar modul (modul di kiri bergantung pada modul di kanan):

```
entities ──▶ core, map, systems
items ──▶ entities, core
map ──▶ core, lib/tileson, systems
systems ──▶ entities, core, ui
rendering ──▶ entities, map, core
ui ──▶ core, systems, rendering
debug ──▶ core, entities, map
utils ──▶ (digunakan oleh systems, rendering)
```

Penjelasan singkat:
- Semua modul bergantung pada `core` sebagai fondasi dasar.
- Modul `entities` dan `map` sering digunakan oleh modul lain untuk logika dunia game.
- Modul `systems` mengorkestrasi logika lintas modul (input, movement, combat).

## Cara Navigasi Kode

1. **Entry Point**: Mulai dari `src/core/main.cpp` untuk memahami alur game utama.
2. **Logika Pemain**: Lihat `src/entities/player.cpp` dan `include/entities/player.h`.
3. **UI**: Semua komponen UI ada di `src/ui/` dan header terkait di `include/ui/`.
4. **Peta**: Logika peta ada di `src/map/` dan dependensi Tileson di `lib/tileson/`.
5. **Menambahkan Fitur Baru**: Buat file `.cpp` di subfolder `src/` yang sesuai, buat header `.h` di `include/` yang sesuai, lalu jalankan `cmake --preset ninja` untuk rekonfigurasi build.

## Konfigurasi Unity Build

- Proyek menggunakan *unity build*: semua file `.cpp` di `src/` digabung menjadi satu unit kompilasi oleh CMake.
- File `.cpp` baru akan otomatis terdeteksi oleh CMake, namun memerlukan rekonfigurasi preset setelah penambahan file.
- Jangan mengubah konfigurasi unity build di `CMakeLists.txt` kecuali diperlukan.
