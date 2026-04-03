# Game RPG - Raylib

Game RPG 2D yang dibuat dengan Raylib.

## Persyaratan

- **Windows** (10/11)
- MSYS2 dengan MinGW-w64 (g++, make)
- PowerShell 7
- Git

### Instalasi MSYS2

1. Install MSYS2 dari [scoop](https://scoop.sh/): `scoop install msys2`
2. Install MinGW-w64 dari MSYS shell: `pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make`

> [!IMPORTANT]
> Proyek ini hanya berjalan di **Windows**. Tidak mendukung Linux atau macOS.

## Setup Pertama Kali

Pada clone pertama, cukup jalankan:

```bash
make app
```

Ini akan:

1. Download Raylib 5.5 ke `lib/raylib/`
2. Compile semua file .cpp
3. Link dengan library
4. Copy raylib.dll ke folder proyek

## Build

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

### Update Raylib

Untuk download ulang Raylib:

```bash
rm -rf lib/raylib
make app
```

## VSCode

1. Install extensions:
   - C/C++ (Microsoft)
   - Makefile Tools (Microsoft)

2. Buka folder proyek
3. Gunakan Makefile Tools untuk build dan run
