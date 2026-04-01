# Game RPG - Raylib

Game RPG 2D yang dibuat dengan Raylib.

## Persyaratan

- MinGW (g++) atau LLVM (clang++)
- [Raylib 5.5](https://www.raylib.com/) (sudah termasuk dalam `lib/`)

## Struktur Proyek

```
.
├── src/                # Kode sumber
│   ├── main.cpp
│   ├── logic.cpp
│   └── dungeon.h
├── lib/                # Library eksternal
│   ├── include/        # Header
│   └── lib/             # Static library + DLL
├── Makefile            # Konfigurasi build
└── .vscode/            # Konfigurasi VSCode
```

## Instruksi Build

### Build dengan GCC (Default)

```bash
make app
```

### Build dengan Clang

```bash
make app-clang
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

> [!NOTE]
> Raylib 5.5 sudah termasuk dalam direktori `lib/`.
