# Kontribusi

## Pengaturan Pengembangan

### 1. Program yang dibutuhkan

- [Cmake](https://cmake.org/)
- [Ninja](https://ninja-build.org/)
- [Ccache](https://ccache.dev/) (optional, namun rekomendasi untuk meningkatkan kecepatan kompilasi lebih lanjut)
- C/C++ Compiler
  - Rekomendasi: [Clang](https://github.com/mstorsjo/llvm-mingw) (Sudah termasuk GCC)

### 2. Instalasi

1. **Windows**
  
   Untuk kemudahan, disarankan menggunakan program manager CLI seperti [Scoop](https://scoop.sh/). Disarankan juga menggunakan PowerShell versi 5.1+.

   Langkah instalasi menggunakan scoop
   1. Install [scoop](https://scoop.sh/)
   2. Install compiler
      1. Rekomendasi: Clang (Sudah termasuk GCC)

         ```powershell
         scoop install mingw-mstorsjo-llvm-ucrt
         ```

   3. Install Cmake, Ninja, dan Ccache
      1. Cmake dan Ninja

         ```powershell
         scoop install cmake ninja
         ```

   4. Install Ccache
      1. Ccache (Optional, namun rekomendasi untuk meningkatkan kecepatan kompilasi lebih lanjut)

         ```powershell
         scoop install ccache
         ```

2. **Unix**

   Beberapa atau bahkan semua program seharusnya sudah terinstall. Apabila ada yang kurang (misalkan ccache atau cmake), silahkan install dengan package manager sesuai dengan sistem operasi.

### 3. Membangun Program

1. Clone repository

   ```bash
   # Tradisional
   git clone https://github.com/COWGRAMMAR/RPG-game-raylib.git

   # GH Cli
   gh repo clone COWGRAMMAR/RPG-game-raylib
   ```

2. Jalankan setup script (PowerShell) untuk mengunduh Raylib dan Tileson

   ```powershell
   .\setup.ps1
   ```

3. Bangun dengan Cmake dan Ninja

   ```powershell
   cmake --preset ninja && cmake --build --preset ninja
   ```

4. Jalankan program

   ```powershell
   # PowerShell
   .\build\bin\main.exe
   ```

   ```bash
   # Bash / CMD
   ./build/bin/main.exe
   ```

5. Menghapus build artifacts

   ```powershell
   # PowerShell
   Remove-Item -Recurse -Force build
   ```

   ```bash
   # CMD
   rmdir /s /q build
   ```

   ```bash
   # Unix
   rm -rf build
   ```

## Pemecahan Masalah

   1. **Error: No such file or directory**
      - Pastikan file yang dibutuhkan sudah terinstall.
      - Jalankan `setup.ps1` untuk mengunduh Raylib dan Tileson.

   2. **Build error setelah menambah file**
      - Jalankan perintah build kembali untuk mengkonfigurasi ulang.
