# Game RPG - Raylib

Game RPG 2D yang dibuat dengan Raylib.

## Persyaratan

- **Windows** (10/11)
- MinGW-w64 dengan g++ (compiler)
- Git Bash atau MSYS2 (untuk menjalankan Makefile)
- [Raylib 5.5](https://www.raylib.com/) (sudah termasuk dalam `lib/`)

### Instalasi MinGW + Git Bash

Install [Git for Windows](https://git-scm.com/) yang sudah termasuk Git Bash. Pastikan `g++` ada di PATH.

> [!IMPORTANT]
> Proyek ini hanya berjalan di **Windows**. Tidak mendukung Linux atau macOS.

## Persyaratan Shell

Proyek ini memerlukan shell yang mendukung perintah Unix (`rm`, `cp`, `mkdir`).

### Shell yang Didukung

| Shell | OS | Status |
|-------|-----|--------|
| Git Bash | Windows | ✅ Didukung |
| MSYS2 | Windows | ✅ Didukung |
| PowerShell 7 | Windows | ✅ Didukung (gunakan `Makefile.ps1`) |
| CMD | Windows | ❌ Tidak didukung |

> [!NOTE]
> Due to a bug in `make`, perintah `cmd /c` yang menjalankan perintah Windows (seperti `copy`, `del`, `mkdir`) dijalankan secara asinkron, menyebabkan kesalahan "The system cannot find the file specified".

### Menggunakan Makefile yang Berbeda

#### Untuk Git Bash / MSYS2 (Default)

```bash
make app
make cln
```

#### Untuk PowerShell 7

```bash
make -f Makefile.ps1 app
make -f Makefile.ps1 cln
```

#### Untuk CMD

Gunakan **Git Bash** atau **PowerShell 7** sebagai gantinya.

## Instruksi Build

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
├── Makefile            # Konfigurasi build (untuk Git Bash)
├── Makefile.ps1        # Konfigurasi build (untuk PowerShell)
├── build/              # File objek (dibuat otomatis)
└── README.md
```

> [!NOTE]
> Raylib 5.5 sudah termasuk dalam direktori `lib/`. Tidak perlu instalasi tambahan.
