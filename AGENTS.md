# RPG Raylib

## Build & Run

```powershell
.\setup.ps1                          # auto-download raylib/tileson/nlohmann-json to lib/
cmake --preset ninja                  # configure (re-run after adding .cpp files)
cmake --build --preset ninja          # build
.\build\bin\main.exe                  # run from project root (NOT by clicking exe)
```

Presets: `ninja` (Release), `ninja-debug`. Uses Unity build (all .cpp in one TU). Prefers clang++, falls back to default. Uses ccache if found.

## Architecture

- Entrypoint: `src/core/main.cpp` → `InitScreen()` → virtual 640x360
- `include/` mirrors `src/` per module: core, entities, items, map, systems, rendering, ui, debug
- Assets auto-copied to build output. raylib.dll auto-copied on Windows.

## Conventions

- Style: PascalCase for multi-word fns/vars, comments required, no magic numbers
- No tests, no linter, no typechecker

Oke, gw baca stylenya. Ini aturannya:

---

**Naming**

- Class/Struct: `PascalCase`
- Function: `PascalCase`
- Variable lokal: `camelCase`
- Member variable: `PascalCase`
- Konstanta/enum: `UPPER_SNAKE_CASE`
- Namespace: `PascalCase`

**Struktur**

- Section divider: `/*=====...=====*/` dengan label
- Satu baris kosong antar function
- Brace `{` selalu di baris yang sama untuk `if/for/while`, baris baru untuk function/class definition
- Single-line `if` tanpa brace boleh kalau bodynya pendek dan jelas

**Komentar**

- Docstring `/** */` hanya untuk function publik yang non-obvious
- Inline comment `//` untuk logika yang butuh penjelasan, bukan yang self-explanatory
- Bahasa komentar: **Indonesia**

**Lain-lain**

- Early return diutamakan daripada nested `if`
- Cast eksplisit pakai `(int)`, `(float)` bukan `static_cast`
- `nullptr` bukan `NULL`
- Inisialisasi member di declaration kalau bisa (`bool x = false`)

## rules
- sebelum melakukan perubahan di workspace konfirmasi dulu apa aja yang diubah mulai dari file mana yang berubah, fungsi apa yang berubah dll.

Context:
- C++ worldgen system, Raylib 5.5
- Corridor stamping: tiap exit object (north/south/east/west) di-stamp corridor prefab dari pool
- Pool size: 10 prefab vertical, 10 prefab horizontal
- Prefab dipilih via SplitMix64 hash dari (worldSeed ^ hash(tileX * 164 + tileY) ^ exitTypeHash)
- exitTypeHash: north=1, south=2, east=3, west=4

Problem:
- Prefab corridor masih sering repeat — prefab yang sama muncul di banyak corridor berbeda
- Sudah dicoba tambah exitTypeHash sebagai differentiator tapi masih repeat

Kode pick function:
uint64_t SplitMix64(uint64_t x)
{
    x += 0x9e3779b97f4a7c15;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
    x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
    return x ^ (x >> 31);
}

int PickCorridorIndex(int tileX, int tileY, int exitTypeHash, int poolSize)
{
    uint64_t hash = SplitMix64(worldSeed ^ SplitMix64((uint64_t)tileX * 164 + (uint64_t)tileY) ^ (uint64_t)exitTypeHash);
    return (int)(hash % (uint64_t)poolSize);
}

Context tambahan:
- worldSeed saat ini placeholder: 12345
- Pool size hanya 10 — kemungkinan collision hash % 10 tinggi
- Exit positions di worldgen map 164×164, tiap cell 41×41 tiles
- Corridor exit selalu berada di posisi absolut tile 21 per axis per cell

Task:
Analisis kenapa distribusi hash masih menghasilkan repeat yang tinggi dengan pool size 5.
Apakah ada masalah di hash function, cara combine input, atau % poolSize yang terlalu kecil?
Sertakan fix yang lebih baik.
