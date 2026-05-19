# Save/Load System

## Architecture: In-Memory Bridge

The system uses **global C++ structs** as a bridge between the game world and disk files:

```txt
[Game World]  <-->  [Global Structs]  <-->  [Disk JSON Files]
                     (in memory)
```

| Function | What it does |
| --- | --- |
| `SaveGameState()` | Reads player/enemies/items/map from game world -> writes to globals |
| `WriteSaveFile(path)` | Takes current globals -> serializes to JSON file |
| `ReadSaveFile(path)` | Reads JSON file -> deserializes into globals |
| `RestoreGameState()` | Takes current globals -> writes back to game world |
| `WriteAutosave(name)` | `SaveGameState()` + `WriteSaveFile()` in one call |

## Save Files on Disk

| File | Trigger | Contents |
| --- | --- | --- |
| `saves/manual/slot0.json` | Pause "Save", Pause "Return to Menu" | Full state |
| `saves/autosave/quick.json` | Map switch | Full state (via `WriteAutosave`) |
| `saves/autosave/periodic.json` | Every 60s in PLAY state | Full state (via `WriteAutosave`) |
| `saves/autosave/spawn.json` | Fresh game (player spawn) | Full state (via `WriteAutosave`) |
| `saves/enemies/<sanitized_map_path>` | Map switch | Per-map enemy positions/HP/death status |
| `saves/items/<sanitized_map_path>` | Map switch | Per-map item pickup state |

## What Gets Saved

Everything in `slot0.json` (single JSON file, version=1):

- **Player**: position (x,y), health, maxHealth, mana, maxMana, hotbar[4] (definitionId, amount), bag[12] (definitionId, amount), animation state (state, direction, isDead), active hotbar slot
- **Enemies**: position, enemyName, currentHP, maxHealth, isAlive, aiState, patrolTarget, patrolTimer, mapObjectID
- **Items**: position, definitionId, isPickedUp
- **Map**: mapPath, cameraTarget (x,y), cameraZoom, deadEntities list, chestsOpened list, mapHistory stack
- **Settings**: showFPS

## All Scenarios

### 1. Fresh Game (No Save File)

Main Menu -> **Start Game** -> LOADING -> Tutorial Map -> PLAY

```txt
Start Game clicked
  +-- HasSaveFile()? NO
  +-- Screen = LOADING
  +-- Loading stages (textures, map, enemies)
  +-- InitAll() -> WriteAutosave("spawn.json") -> PLAY
```

- No confirmation popup (nothing to overwrite)
- `spawn.json` autosave created at `saves/autosave/spawn.json`

### 2. Fresh Game (With Existing Save)

Main Menu -> **Start Game** -> "Overwrite existing save?" popup

```txt
Start Game clicked
  +-- HasSaveFile()? YES
  +-- Show two-button popup: [Start New] [Cancel]
  +-- Click [Start New]:
       DeleteSaveFile -> ClearSavedState -> LOADING
  +-- Click [Cancel]:
       Popup hides, back to main menu
```

- Save file deleted when you confirm
- Then behaves exactly like Scenario 1

### 3. Playing: Manual Save

Press grave key -> Pause Menu -> **Save Game**

```txt
Save Game clicked
  +-- SaveGameState() -> populates globals from game world
  +-- WriteSaveFile("saves/manual/slot0.json") -> writes to disk
  +-- "Game Saved!" popup appears
```

- Saves everything to `saves/manual/slot0.json`

### 4. Playing: Load Game from Pause

Press grave key -> Pause Menu -> **Load Game**

```txt
Load Game clicked
  +-- HasSaveFile()?
       YES -> Show confirmation popup: [Load Save] [Cancel]
       +-- Click [Load Save]:
            ReadSaveFile() -> screen = LOADING
            +-- Fast path: InitAll() -> RestoreGameState() -> PLAY
       +-- Click [Cancel]:
            Back to pause menu
       NO  -> "No save file found" popup
```

- **WARNING**: Current progress is replaced by the saved state
- No auto-save before loading (current state is lost)

### 5. Playing: Return to Main Menu

Press grave key -> Pause Menu -> **Return to Main Menu**

```txt
Return to Menu clicked
  +-- SaveGameState() + WriteSaveFile("saves/manual/slot0.json")
  +-- Screen = MAIN_MENU
```

- **Auto-saves to slot0.json**
- If you then click **Load Game** from main menu, it loads this auto-saved state
- There is no way to "return to menu without saving" currently

### 6. Main Menu: Load Game

Main Menu -> **Load Game**

```txt
Load Game clicked
  +-- HasSaveFile()?
       NO  -> "No save file" popup, click OK, back to menu
       YES -> ReadSaveFile() -> show confirmation popup
              +-- Click [Load Save]:
                   screen = LOADING
                   +-- Fast path: InitAll() -> RestoreGameState() -> PLAY
              +-- Click [Cancel]:
                   ClearSavedState() -> back to menu
```

### 7. Map Switching (Door to New Map)

Enter door in game world -> LOADING -> New Map -> PLAY

```txt
Enter door in game world
  +-- SwitchMap() saves current map's enemies + items to per-map files
  +-- LOADING screen:
       Stage 0: UnloadMap
       Stage 1: LoadMap(new map), SetCurrentMapPath
       Stage 2: Clear entity registry
                LoadEnemiesForMap(new map)?
                  NO  -> SpawnEnemiesFromMap() from spawn points
                  YES -> Enemies restored from saved state
                LoadItemsForMap(new map)?
                  NO  -> SpawnItemWave()
                  YES -> Items restored from saved state
       Stage 3: Init player at target door, WriteAutosave("quick.json")
                -> PLAY
```

- Dead entities that were killed on this map before are tracked in the `DeadEntities` set
- `SpawnEnemiesFromMap()` skips spawn points where `IsAlreadyDead()` returns true
- Per-map enemy/items files are how state persists on maps you've visited before

### 8. Map Switching (Return to Previous Map)

Same flow as #7. `LoadEnemiesForMap()` finds the per-map file from your last visit, restoring enemies to their previous state (dead stay dead, alive restored with HP/position/AI state).

### 9. Corrupted Save File

```txt
Load Game clicked
  +-- ReadSaveFile() tries to parse JSON
  +-- json::parse_error exception caught
  +-- Returns false
  +-- mainMenu: deletes corrupted file, shows "corrupted" popup
  +-- Click OK -> back to main menu
```

- File is **deleted** to prevent repeated errors
- No repair attempt -- corrupted = deleted

### 10. Version Mismatch

Same flow as #9. `ReadSaveFile()` checks `version == 1`. If not, returns false.

### 11. 60-Second Periodic Autosave

While in PLAY state, every 60 seconds:

```txt
autosaveTimer += frameTime
if (timer >= 60.0f) {
    WriteAutosave("periodic.json")
    timer = 0
}
```

- Timer **pauses** when pause menu is active
- Timer **resets** on manual save

### 12. Save Error (Disk Full / Permission)

```txt
Pause "Save Game" clicked
  +-- WriteSaveFile returns false
  +-- "Failed to save game. Check disk space." error popup
```

### 13. Load When Save References Missing Map

```txt
RestoreGameState() checks savedMapState.mapPath
  +-- File exists?
       NO  -> Fallback to "assets/maps/tutorial.json" with warning log
       YES -> Load normally
```

## Summary: What Saves When

| Event | What gets persisted |
| --- | --- |
| Pause **Save Game** | Everything to `slot0.json` |
| Pause **Return to Menu** | Everything to `slot0.json` (auto) |
| **Map switch** | Current map enemies/items to per-map files + full state to `quick.json` |
| **60s timer** | Full state to `periodic.json` |
| **Fresh game start** | Full state to `spawn.json` |
| **Main menu Load Game** | Reads `slot0.json`, restores everything |

## Key Source Files

| File | Role |
| --- | --- |
| `src/core/game_state_saver.cpp` | Core save/load logic, JSON I/O, autosave |
| `include/core/game_state_saver.h` | Global state structs and function declarations |
| `src/ui/mainMenu.cpp` | Main menu save-aware Start Game + Load Game |
| `src/ui/pauseMenu.cpp` | Pause menu Save/Load/Return to Menu |
| `src/core/loading_screen.cpp` | Loading screen integration, map switch stages |
| `src/map/map.cpp` | Map switching functions (SwitchMap, GoBack, InitMap) |
| `src/entities/enemies/enemy.cpp` | Per-map enemy persistence (SaveEnemiesForMap, LoadEnemiesForMap, SpawnEnemiesFromMap) |
| `src/entities/entities.cpp` | Dead entity registry (RegisterDeath, IsAlreadyDead) |
| `src/ui/popup.cpp` | Two-button confirmation popup |
