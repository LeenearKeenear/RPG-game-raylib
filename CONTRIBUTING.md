# Kontribusi

## Pengaturan Pengembangan

### Alat yang Diperlukan

Pasang alat-alat berikut untuk membangun proyek:

| Alat | Windows (scoop) | macOS (brew) | Linux (apt) |
|------|-----------------|--------------|-------------|
| **Compiler (gcc)** | `scoop install gcc` | `brew install gcc` | `apt install gcc` |
| **CMake** | `scoop install cmake` | `brew install cmake` | `apt install cmake` |
| **Ninja** | `scoop install ninja` | `brew install ninja` | `apt install ninja-build` |
| **ccache** | `scoop install ccache` | `brew install ccache` | `apt install ccache` |

### Pengaturan Pertama

1. Pasang semua alat yang diperlukan (lihat tabel di atas)
2. Jalankan skrip setup untuk mengunduh raylib:
   ```powershell
   .\setup.ps1
   ```

## Membangun

### Mulai Cepat

```bash
# Configure (sekali saja, atau setelah menambah file baru)
cmake --preset ninja

# Build
cmake --build --preset ninja
```

File executable akan berada di `build/bin/main.exe`.

### Preset Build

| Preset | Deskripsi |
|--------|-----------|
| `ninja` | Build release dengan optimasi (default) |
| `ninja-debug` | Build debug dengan simbol |

### Build Manual (tanpa preset)

```bash
cmake -B build -G Ninja
cmake --build build --parallel
```

### Build Bersih

```bash
# PowerShell
Remove-Item -Recurse -Force build

# CMD
rmdir /s /q build

# Kemudian build ulang
cmake --preset ninja
cmake --build --preset ninja
```

### Referensi Perintah

| Aksi | PowerShell | CMD |
|------|------------|-----|
| **Configure** | `cmake --preset ninja` | `cmake --preset ninja` |
| **Build** | `cmake --build --preset ninja` | `cmake --build --preset ninja` |
| **Clean** | `Remove-Item -Recurse -Force build` | `rmdir /s /q build` |

## Menambahkan File Sumber Baru

File `.cpp` baru di `src/` akan otomatis ditemukan pada saat CMake berjalan ulang. Tidak perlu perubahan manual.

## Pemecahan Masalah

- **Error "No such file or directory"**: Jalankan `.\setup.ps1` untuk mengunduh dependensi
- **Build error setelah menambah file**: Jalankan `cmake --preset ninja` untuk mengkonfigurasi ulang
