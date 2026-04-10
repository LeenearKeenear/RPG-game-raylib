# Game RPG - Raylib

Game RPG 2D yang dibuat dengan Raylib.

## Fitur

- Main menu dengan button components
- Player movement system
- Tilemap support (Tileson JSON)
- Virtual screen rendering (640x360)

## Persyaratan

- **OS**: Windows 10/11, macOS, atau Linux
- **Compiler**: gcc atau clang
- **CMake**: >= 3.20
- **Ninja**
- **Git**

## Dependencies

- **Raylib 5.5**: Auto-download via `setup.ps1` (Windows) / Manual (macOS/Linux)
- **Tileson**: Download manual dari [GitHub Tileson](https://github.com/SSBMTonberry/tileson), copy `tileson.hpp` ke `lib/tileson/`

## Setup Pertama Kali

```bash
cmake --preset ninja
cmake --build --preset ninja
```

Ini akan:
1. Download Raylib 5.5 ke `lib/raylib/` (Windows saja)
2. Compile semua file .cpp (Unity build)
3. Link dengan library
4. Copy raylib.dll ke folder output

## Build

```bash
# Build release (default)
cmake --build --preset ninja

# Build debug
cmake --build --preset ninja-debug
```

### Jalankan Game

```bash
./build/bin/main.exe
```

## Catatan

Untuk panduan lengkap (termasuk cara setup alat per-platform, command reference CMD/PowerShell, troubleshooting), lihat [CONTRIBUTING.md](./CONTRIBUTING.md).
