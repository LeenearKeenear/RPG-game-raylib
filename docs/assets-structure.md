# Struktur Folder Assets

Dokumentasi ini menjelaskan struktur folder `assets/` yang menyimpan semua aset non-kode proyek RPG Game Raylib, hasil restrukturisasi terbaru.

## Pohon Folder Assets

```
assets/
├── audio/                # Aset audio game
│   └── sfx/              # Efek suara (Sound Effects)
│       └── 666herohero-slash-21834.mp3
├── textures/             # Aset gambar (sprites, tiles, UI)
│   ├── autotiles.png     # Tile otomatis untuk pemetaan
│   ├── enemies.png       # Sprite sheet musuh
│   ├── knight.png        # Sprite sheet pemain (knight)
│   ├── logo.png          # Logo game
│   ├── test.png          # Tekstur uji coba
│   └── tiles.png         # Tile sheet peta
└── maps/                 # File peta game (format JSON Tileson)
    ├── floorA.json       # Peta lantai A
    ├── floorB.json       # Peta lantai B
    ├── floorC.json       # Peta lantai C
    └── tutorial.json      # Peta tutorial pemain
```

## Daftar File per Kategori

### Audio (audio/)
| File | Deskripsi |
|------|-----------|
| `sfx/666herohero-slash-21834.mp3` | Efek suara serangan pedang (slash) |

### Tekstur (textures/)
| File | Deskripsi |
|------|-----------|
| `autotiles.png` | Tile otomatis untuk pengisian area peta secara otomatis |
| `enemies.png` | Sprite sheet berisi animasi semua jenis musuh |
| `knight.png` | Sprite sheet karakter pemain (kelas Knight) |
| `logo.png` | Logo resmi game, digunakan di menu utama |
| `test.png` | Tekstur sementara untuk uji coba fitur baru |
| `tiles.png` | Sprite sheet tile dasar untuk pembuatan peta |

### Peta (maps/)
| File | Deskripsi |
|------|-----------|
| `floorA.json` | Data peta lantai pertama game |
| `floorB.json` | Data peta lantai kedua game |
| `floorC.json` | Data peta lantai ketiga game |
| `tutorial.json` | Data peta tutorial untuk pemain baru |

## Format File Aset

| Jenis Aset | Format | Keterangan |
|------------|--------|------------|
| Audio | MP3 | Efek suara pendek, dikelola di folder `audio/sfx/` |
| Tekstur | PNG | Gambar tanpa kompresi lossy, mendukung transparansi |
| Peta | JSON | Format Tileson (kompatibel dengan Tiled Map Editor) |

## Cara Navigasi Aset

1. **Audio**: Tambahkan efek suara baru ke `assets/audio/sfx/`, pastikan format MP3.
2. **Tekstur**: Tambahkan sprite/tiles baru ke `assets/textures/`, gunakan format PNG.
3. **Peta**: Buat peta baru menggunakan Tiled, ekspor ke format JSON Tileson, simpan di `assets/maps/`.
