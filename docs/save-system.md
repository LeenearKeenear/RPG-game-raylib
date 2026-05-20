# Sistem Save/Load

## Arsitektur: In-Memory Bridge

Sistem ini menggunakan **struct global C++** sebagai jembatan antara game world dan file di disk:

```txt
[Game World]  <-->  [Global Structs]  <-->  [File JSON di Disk]
                     (di memori)
```

| Fungsi | Apa yang dilakukan |
| --- | --- |
| `SaveGameState()` | Baca player/musuh/item/map dari game world -> tulis ke global structs |
| `WriteSaveFile(path)` | Ambil global structs saat ini -> serialisasi ke file JSON |
| `ReadSaveFile(path)` | Baca file JSON -> deserialisasi ke global structs |
| `RestoreGameState()` | Ambil global structs -> tulis balik ke game world |
| `WriteAutosave(name)` | `SaveGameState()` + `WriteSaveFile()` dalam satu panggilan |

## File Save di Disk

| File | Pemicu | Isi |
| --- | --- | --- |
| `saves/manual/slot0.json` | Pause "Save", Pause "Return to Menu" | Full state |
| `saves/autosave/quick.json` | Map switch | Full state (via `WriteAutosave`) |
| `saves/autosave/periodic.json` | Setiap 60 detik di PLAY state | Full state (via `WriteAutosave`) |
| `saves/autosave/spawn.json` | Fresh game (player spawn) | Full state (via `WriteAutosave`) |
| `saves/enemies/<sanitized_map_path>` | Map switch | Posisi/HP/status mati musuh per-map |
| `saves/items/<sanitized_map_path>` | Map switch | Status pickup item per-map |

## Apa yang Disimpan

Semua ada di `slot0.json` (satu file JSON, version=1):

- **Player**: position (x,y), health, maxHealth, mana, maxMana, hotbar[4] (definitionId, amount), bag[12] (definitionId, amount), animation state (state, direction, isDead), active hotbar slot
- **Musuh**: position, enemyName, currentHP, maxHealth, isAlive, aiState, patrolTarget, patrolTimer, mapObjectID
- **Item**: position, definitionId, isPickedUp
- **Map**: mapPath, cameraTarget (x,y), cameraZoom, deadEntities list, chestsOpened list, mapHistory stack
- **Settings**: showFPS

## Semua Skenario

### 1. Fresh Game (Tanpa Save File)

Main Menu -> **Start Game** -> LOADING -> Tutorial Map -> PLAY

```txt
Start Game diklik
  +-- HasSaveFile()? TIDAK
  +-- Screen = LOADING
  +-- Loading stages (texture, map, musuh)
  +-- InitAll() -> WriteAutosave("spawn.json") -> PLAY
```

- Tidak ada popup konfirmasi (tidak ada yang di-overwrite)
- Autosave `spawn.json` dibuat di `saves/autosave/spawn.json`

### 2. Fresh Game (Dengan Save File yang Sudah Ada)

Main Menu -> **Start Game** -> popup "Overwrite existing save?"

```txt
Start Game diklik
  +-- HasSaveFile()? YA
  +-- Tampilkan popup dua-tombol: [Start New] [Cancel]
  +-- Klik [Start New]:
       DeleteSaveFile -> ClearSavedState -> LOADING
  +-- Klik [Cancel]:
       Popup disembunyikan, kembali ke main menu
```

- Save file dihapus saat konfirmasi
- Setelah itu berjalan persis seperti Skenario 1

### 3. Bermain: Manual Save

Tekan tombol grave -> Pause Menu -> **Save Game**

```txt
Save Game diklik
  +-- SaveGameState() -> mengisi global structs dari game world
  +-- WriteSaveFile("saves/manual/slot0.json") -> tulis ke disk
  +-- Popup "Game Saved!" muncul
```

- Menyimpan semua ke `saves/manual/slot0.json`

### 4. Bermain: Load Game dari Pause

Tekan tombol grave -> Pause Menu -> **Load Game**

```txt
Load Game diklik
  +-- HasSaveFile()?
       YA -> Tampilkan popup konfirmasi: [Load Save] [Cancel]
       +-- Klik [Load Save]:
            ReadSaveFile() -> screen = LOADING
            +-- Fast path: InitAll() -> RestoreGameState() -> PLAY
       +-- Klik [Cancel]:
            Kembali ke pause menu
       TIDAK -> Popup "No save file found"
```

- **PERINGATAN**: Progress saat ini digantikan oleh saved state
- Tidak ada auto-save sebelum loading (state saat ini hilang)

### 5. Bermain: Return to Main Menu

Tekan tombol grave -> Pause Menu -> **Return to Main Menu**

```txt
Return to Menu diklik
  +-- Popup konfirmasi: "Return to main menu?" + sub-message
  +-- Klik [Confirm]:
       SaveGameState() + WriteSaveFile("saves/manual/slot0.json")
       Screen = MAIN_MENU
```

- **Auto-save ke slot0.json**
- Jika kemudian klik **Load Game** dari main menu, akan load state yang auto-saved ini
- Tidak ada cara untuk "return to menu tanpa save" saat ini

### 6. Main Menu: Load Game

Main Menu -> **Load Game**

```txt
Load Game diklik
  +-- HasSaveFile()?
       TIDAK -> Popup "No save file", klik OK, kembali ke menu
       YA -> ReadSaveFile() -> tampilkan popup konfirmasi
              +-- Klik [Load Save]:
                   screen = LOADING
                   +-- Fast path: InitAll() -> RestoreGameState() -> PLAY
              +-- Klik [Cancel]:
                   ClearSavedState() -> kembali ke menu
```

### 7. Map Switching (Pintu ke Map Baru)

Masuk pintu di game world -> LOADING -> Map Baru -> PLAY

```txt
Masuk pintu di game world
  +-- SwitchMap() menyimpan musuh + item map saat ini ke file per-map
  +-- Loading screen:
       Stage 0: UnloadMap
       Stage 1: LoadMap(map baru), SetCurrentMapPath
       Stage 2: Clear entity registry
                LoadEnemiesForMap(map baru)?
                  TIDAK -> SpawnEnemiesFromMap() dari spawn points
                  YA -> Musuh direstore dari saved state
                LoadItemsForMap(map baru)?
                  TIDAK -> SpawnItemWave()
                  YA -> Item direstore dari saved state
       Stage 3: Init player di target door, WriteAutosave("quick.json")
                -> PLAY
```

- Entitas mati yang sudah dibunuh di map ini sebelumnya dilacak di set `DeadEntities`
- `SpawnEnemiesFromMap()` melewati spawn point yang `IsAlreadyDead()` mengembalikan true
- File musuh/item per-map adalah cara state bertahan di map yang pernah dikunjungi sebelumnya

### 8. Map Switching (Kembali ke Map Sebelumnya)

Sama seperti #7. `LoadEnemiesForMap()` menemukan file per-map dari kunjungan terakhir, merestore musuh ke state sebelumnya (mati tetap mati, hidup direstore dengan HP/posisi/AI state).

### 9. Save File Corrupt

```txt
Load Game diklik
  +-- ReadSaveFile() mencoba parse JSON
  +-- Exception json::parse_error tertangkap
  +-- Mengembalikan false
  +-- mainMenu: hapus file corrupt, tampilkan popup "corrupted"
  +-- Klik OK -> kembali ke main menu
```

- File **dihapus** untuk mencegah error berulang
- Tidak ada upaya perbaikan -- corrupt = dihapus

### 10. Version Mismatch

Sama seperti #9. `ReadSaveFile()` memeriksa `version == 1`. Jika tidak, mengembalikan false.

### 11. Periodic Autosave 60 Detik

Saat di PLAY state, setiap 60 detik:

```txt
autosaveTimer += frameTime
if (timer >= 60.0f) {
    WriteAutosave("periodic.json")
    timer = 0
}
```

- Timer **berhenti** saat pause menu aktif
- Timer **di-reset** saat manual save

### 12. Save Error (Disk Penuh / Permission)

```txt
Pause "Save Game" diklik
  +-- WriteSaveFile mengembalikan false
  +-- Popup error "Failed to save game. Check disk space."
```

### 13. Load Saat Save Merujuk ke Map yang Tidak Ada

```txt
RestoreGameState() memeriksa savedMapState.mapPath
  +-- File ada?
       TIDAK -> Fallback ke "assets/maps/tutorial.json" dengan warning log
       YA -> Load secara normal
```

## Ringkasan: Apa yang Tersimpan Kapan

| Event | Apa yang dipersisten |
| --- | --- |
| Pause **Save Game** | Semua ke `slot0.json` |
| Pause **Return to Menu** | Semua ke `slot0.json` (otomatis) |
| **Map switch** | Musuh/item map saat ini ke file per-map + full state ke `quick.json` |
| **Timer 60 detik** | Full state ke `periodic.json` |
| **Fresh game start** | Full state ke `spawn.json` |
| **Main menu Load Game** | Baca `slot0.json`, restore semua |

## Known Issues / Notices

> Daftar bug yang diketahui dan catatan penting tentang sistem save/load saat ini.

| Issue | Status | Detail |
| --- | --- | --- |
| Crash saat load save dari map berbeda | Intermiten | Terjadi secara tidak konsisten saat loading save yang dibuat di map berbeda dari map saat ini. Belum bisa direproduksi secara andal. Kemungkinan terkait dengan enemy/item restore yang merujuk ke entitas yang belum di-spawn. |
| Musuh tidak sepenuhnya stabil | Known | Sistem musuh masih setengah jadi. Perilaku AI dan per-map persistence mungkin tidak selalu konsisten. |

## File Source Utama

| File | Peran |
| --- | --- |
| `src/core/game_state_saver.cpp` | Logika save/load inti, JSON I/O, autosave |
| `include/core/game_state_saver.h` | Struct state global dan deklarasi fungsi |
| `src/ui/mainMenu.cpp` | Main menu save-aware Start Game + Load Game |
| `src/ui/pauseMenu.cpp` | Pause menu Save/Load/Return to Menu |
| `src/core/loading_screen.cpp` | Integrasi loading screen, stage map switch |
| `src/map/map.cpp` | Fungsi map switching (SwitchMap, GoBack, InitMap) |
| `src/entities/enemies/enemy.cpp` | Per-map enemy persistence (SaveEnemiesForMap, LoadEnemiesForMap, SpawnEnemiesFromMap) |
| `src/entities/entities.cpp` | Dead entity registry (RegisterDeath, IsAlreadyDead) |
| `src/ui/popup.cpp` | Two-button confirmation popup |
