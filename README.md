# Game RPG - Raylib

Game RPG 2D yang dibuat dengan Raylib.

## Persyaratan

- **Windows** (10/11)
- MinGW-w64 dengan g++ (compiler)
- [Raylib 5.5](https://www.raylib.com/) (sudah termasuk dalam `lib/`)

### Instalasi MinGW

Jika belum punya MinGW, install salah satu:

1. **Scoop** (direkomendasikan):
   ```powershell
   scoop install mingw
   ```

2. **Manual**:
   - Download [MinGW-w64](https://www.mingw-w64.org/)
   - Pastikan `g++` ada di PATH

> [!IMPORTANT]
> Proyek ini hanya berjalan di **Windows**. Tidak mendukung Linux atau macOS.

## Instruksi Build

Pastikan terminal mendukung perintah Unix (`rm`, `cp`, `mkdir`).
Jika menggunakan **PowerShell/CMD native**, install [Git for Windows](https://git-scm.com/) yang sudah termasuk Git Bash.

### Build

```bash
make app
```

### Bersihkan Build

```bash
make cln
```

### Jalankan Game

```bash
./main.exe
```

## Setup di VSCode

1. Install extensions:
   - C/C++ (Microsoft)
   - Makefile Tools (microsoft)

2. Buka folder proyek
3. Gunakan Makefile Tools untuk build dan run

## Struktur Proyek

```
.
├── src/                # Kode sumber
├── include/            # Header proyek
├── lib/                # Library dan header raylib
├── Makefile            # Konfigurasi build
└── build/              # File objek (dibuat otomatis)
```

> [!NOTE]
> Raylib 5.5 sudah termasuk dalam direktori `lib/`. Tidak perlu instalasi tambahan.
