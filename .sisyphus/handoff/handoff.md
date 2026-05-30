# Handoff: Keybinds Tab Reorganization & Font Change

## Session Context

This handoff covers analysis and implementation planning for two tasks on the RPG-raylib project (raylib C++ dungeon game, 1280x720 virtual screen). Full codebase analysis was done in the original session -- all findings are captured here.

## Priority Order

**Step 1 first** (reorganize keybinds), then **Step 2** (change font). They are independent but doing fonts after the UI reorganization means you only touch `DrawText` calls once with the correct layout.

---

## Step 1: Reorganize Keybinds Into Categories (Main / Debugging)

### Current State

File: `src/ui/keybindsTab.cpp`

Two hardcoded arrays of 18 strings each, rendered in 2 columns of 9 rows. The display is **partially inaccurate** -- "F" is shown for Interact but actual code in `src/systems/input.cpp:26` uses `KEY_E`.

### Files to Modify

| File | Change |
|------|--------|
| `src/ui/keybindsTab.cpp` | Complete rewrite of data + rendering |
| `include/ui/keybindsTab.h` | No change needed (signature stays same) |

### Complete Action-to-Key Mapping (for the reorganized display)

This is the **actual** mapping from the codebase (NOT what keybindsTab currently shows):

#### Main (core gameplay -- 13 items)

| # | Actual Key | Action Name | Code Location | Notes |
|---|-----------|-------------|---------------|-------|
| 1 | W / Arrow Up | Move Up | `input.cpp:20` | |
| 2 | S / Arrow Down | Move Down | `input.cpp:21` | |
| 3 | A / Arrow Left | Move Left | `input.cpp:22` | |
| 4 | D / Arrow Right | Move Right | `input.cpp:23` | |
| 5 | E | Interact | `input.cpp:26` | **BUG:** display says "F", fix to "E" |
| 6 | I | Inventory | `input.cpp:28` | |
| 7 | M | Map | `input.cpp:29` | |
| 8 | Mouse Left | Action / Attack | `input.cpp:33`, `combat.cpp:62` | |
| 9 | 1 | Weapon 1 | `input.cpp:41` | |
| 10 | 2 | Weapon 2 | `input.cpp:42` | |
| 11 | 3 | Potion 1 | `input.cpp:43` | |
| 12 | 4 | Potion 2 | `input.cpp:44` | |
| 13 | Scroll | Hotbar Slot | `input.cpp:48,97-104` | Two behaviors: hotbar scroll + debug zoom |

#### Debugging (non-core / dev tools -- 5 items)

| # | Actual Key | Action Name | Code Location | Notes |
|---|-----------|-------------|---------------|-------|
| 1 | ` (Grave) | Pause Menu | `main.cpp:116` | Toggle pause menu, NOT in input.cpp |
| 2 | Tab | Debug Overlay | `debugmode.cpp:296` | Toggle debug mode, NOT in input.cpp |
| 3 | R | Revive | `input.cpp:27`, `player.cpp:106` | In-game revive mechanic |
| 4 | K | Damage (Test) | `input.cpp:46` | **DEAD CODE** -- `testLoseHP` has ZERO consumers |
| 5 | B | Previous Map | `input.cpp:49`, `player.cpp:135` | Map transition debug |

### Additional Keys NOT Displayed But Used

These exist in code but have no display entry:

| Key | Action | Location | Suggested Category |
|-----|--------|----------|-------------------|
| Q | Drop Item | `input.cpp:30` | Main |
| Left Ctrl / Right Ctrl | Drop All / Modifier | `input.cpp:31,39` | Main |
| Mouse Right | Dash / Drink Potion / Split | `movement.cpp:53`, `inventory.cpp:45` | Main |
| Q + Ctrl | Drop All Items | `player.cpp:342` | Main |

These could optionally be added to the new display.

### Layout Options for the Options Panel

The options screen is 800x600, centered on 1280x720. Content starts at `startY + 100`. Current layout uses 2 hardcoded columns (col1X=`startX+40`, col2X=`startX+450`).

**Suggested approach for categorized layout:**

Option A: Two sub-headers ("=== MAIN ===" and "=== DEBUG ===") in a single column, scrolling if needed. The 13 main + 5 debug items fit in ~18 rows x 28px = 504px, within the 500px available height.

Option B: Main in left column, Debug in right column with section headers.

**Recommendation: Option A** (single column with section headers) because:
- Avoids uneven column split (13 vs 5)
- More readable than two-column with different lengths
- Clear visual separation with headers and different colors

### Implementation Notes

- Use `static const` arrays with structs instead of two parallel arrays (clearer code)
- Fix the "F" -> "E" discrepancy for Interact
- Optionally add missing keys (Q, Ctrl, Mouse Right)
- Optionally remove or mark the dead K key action as "(No Effect)"
- Colors: Main section headers in YELLOW, Debug headers in a muted color (e.g., ORANGE or GRAY)
- All `DrawText` calls use raylib default font at size 20

### Suggested Code Structure

```cpp
struct KeybindEntry {
    const char* key;
    const char* action;
};

// Then split into two arrays:
static const std::array<KeybindEntry, 13> mainKeybinds = { ... };
static const std::array<KeybindEntry, 5> debugKeybinds = { ... };
```

---

## Step 2: Change Font Used Across the Game

### Current Font Architecture

**Zero custom fonts are loaded.** The entire game uses raylib's built-in default bitmap font via `DrawText()` and `MeasureText()`. There are no `LoadFont()`, `GetFontDefault()`, `.ttf`/`.otf` references anywhere.

### Scope: 52 `DrawText()` Calls Across 11 Files

| File | Count | Sizes Used | Context |
|------|-------|-----------|---------|
| `src/ui/keybindsTab.cpp` | 6 | 20 | Keybind list |
| `src/ui/videoTab.cpp` | 2 | 24 | Fullscreen/FPS labels |
| `src/ui/audioTab.cpp` | 7 | 18, 24 | Volume labels |
| `src/ui/popup.cpp` | 1 | 30 | Popup messages |
| `src/ui/pauseMenu.cpp` | uses `buttonTxt` | 30 | Menu buttons (uses Button template) |
| `src/ui/mainMenu.cpp` | uses `buttonTxt` | 30 | Main menu buttons |
| `src/debug/debugmode.cpp` | 28 | 10-18 | Debug panels |
| `src/core/screen_handler.cpp` | 1 | 20 | FPS counter |
| `src/core/loading_screen.cpp` | 2 | 20 | Loading text + progress |
| `src/rendering/hud.cpp` | 2 | 12, variable | HUD labels (also uses `DrawTextHUD` wrapper) |
| `src/systems/effects.cpp` | 3 | 10 | Damage popups + log |
| `src/entities/player.cpp` | 2 | 10 | Interact prompt "[E] Interact" |
| `src/items/item.cpp` | 1 | 8 | Item stack amounts |

Plus `include/ui/button.h:193` -- the `Button<TextPolicy>::Render()` method calls `DrawText()`. This is used by all `buttonTxt` instances (~15 buttons across menus).

### Text Rendering Abstractions

1. **Direct `DrawText()`** -- 45+ calls across all files
2. **`DrawTextHUD()`** in `src/rendering/hud.cpp:82-91` -- wraps `DrawText` with rounded-rect background
3. **`Button<TextPolicy>::Render()`** in `include/ui/button.h:189-193` -- wraps `DrawText` in button template

### Font Loading: Where to Add It

**Pattern to follow:** `InitTextures()` in `src/rendering/animation.cpp:105-116` is where all game textures are loaded. Font loading should go here or in a new `InitFonts()` function called from the same loading stage.

**Loading screen location:** `src/core/loading_screen.cpp:188-193` -- Stage 0 calls `InitTextures()`. Add font loading in a new Stage or within `InitTextures()`.

**Unloading location:** `GameShutDown()` in `src/core/screen_handler.cpp:388-398` -- add `UnloadFont()` after `CloseTextures()`.

**Global font storage:** Options:
- Add a `Font` field to `GameState` in `include/core/screen.h`
- Create a global `Font` variable (like `gState` pattern)
- Create a FontManager class

**Recommendation:** Use a global `Font` variable declared in a new `include/rendering/fonts.h` header, matching the existing globals pattern (e.g., `extern Font gameFont;`).

### Available Fonts (from D:\My Documents\Fonts)

The folder contains many `.ttf` and `.otf` font files (raylib's `LoadFont()` supports both). Best candidates for a game UI:

| Font Path | Size | Style | Best For |
|-----------|------|-------|----------|
| `_Nerd Font/JetBrainsMono/JetBrainsMonoNerdFont-Regular.ttf` | 2.4MB | Clean monospace | Debug panels, FPS counter, keybinds |
| `VES Fonts/Poppins-Regular.ttf` | 158KB | Modern sans-serif | HUD, menus, general UI |
| `VES Fonts/Roboto-Regular.ttf` | 126KB | Classic sans-serif | Any UI (safe choice) |
| `VES Fonts/Poppins-Bold.ttf` | 154KB | Bold sans-serif | Headers, section titles |
| `MiSans-Bold.ttf` | 7.6MB | Bold Chinese/Latin | Headers, prominent text |
| `NewDawn (demo)/NewDawn (demo).ttf` | 29KB | Fantasy/dungeon style | RPG-themed headings |
| `Qanelas Soft FREE/` | (directory) | Soft rounded | Buttons, friendly UI |
| `brotesque/Brotesque Regular.ttf` | 20KB | Decorative | Small labels |

**Note on .otf vs .ttf:** raylib's `LoadFont()` handles both formats identically. The `.otf` files in `/VES Fonts/` (Poppins, Roboto, Lato families) are equally valid -- just use the full path as the font file argument.

**Recommended approach:** Pick 2 fonts -- one for UI/body text (Poppins or Roboto) and one for decorative headings (NewDawn or Qanelas Soft). This follows the pattern where different sizes are already used for different elements.

### Conversion: `DrawText` -> `DrawTextEx`

Every `DrawText(text, x, y, size, color)` call becomes:
```cpp
DrawTextEx(font, text, Vector2{(float)x, (float)y}, (float)size, 0, color);
```

The `MeasureText(text, size)` calls become:
```cpp
MeasureTextEx(font, text, (float)size, 0).x  // returns Vector2, use .x for width
```

### `buttonTxt` Impact

`Button<TextPolicy>::Render()` in `include/ui/button.h:193` calls `DrawText()`. **This file will need a font parameter or a global font reference.** Options:
1. Pass `Font` to the `TextPolicy` constructor (breaking change to all button constructions)
2. Use a global font variable (simpler, matches codebase style)

**Recommendation:** Use a global `Font` variable (`extern Font gameFont;`) and update `button.h` to use it. This minimizes changes across all menu files.

### CMakeLists Note

Unity build. New `.cpp` files (like a `fonts.cpp`) require cmake reconfigure:
```
cmake --preset ninja && cmake --build --preset ninja
```

Existing files edited only (no new `.cpp`) = no reconfigure needed. Adding a new header-only file (`fonts.h`) = no reconfigure needed as long as no new `.cpp`.

### Build & Test

```powershell
# After changes
cmake --build --preset ninja

# Run
.\build\bin\main.exe
```

Build from project root: `C:\Users\Epb\Documents (Unsynced)\Work Folders\Projects\College\RPG-game-raylib`

---

## Files Referenced in This Analysis

### Core Architecture
- `include/core/screen.h` -- GameState struct, ScreenState enum, virtual screen constants
- `src/core/screen_handler.cpp` -- InitScreen(), GameShutDown(), DrawText FPS counter
- `src/core/loading_screen.cpp` -- Asset loading stages, InitTextures() call
- `src/core/main.cpp` -- Game loop, `IsKeyPressed(KEY_GRAVE)` pause toggle

### Input System
- `include/systems/input.h` -- InputState struct, PlayerInput class, PlayerAction enum
- `src/systems/input.cpp` -- Hardcoded keybinds in PollInput(), UpdateState(), ResolveAction()

### UI System
- `include/ui/pauseMenu.h` -- OptionsScreen class (tabs container, buttonTxt members)
- `src/ui/pauseMenu.cpp` -- OptionsScreen Update/Draw, tab switching, CalculateDimensions()
- `include/ui/button.h` -- Button<TextPolicy> template, `buttonTxt` alias, `DrawText()` in Render()
- `include/ui/keybindsTab.h` -- DrawKeybindsTab() declaration
- `src/ui/keybindsTab.cpp` -- Current hardcoded keybind display (**file to rewrite**)
- `include/ui/videoTab.h` / `src/ui/videoTab.cpp` -- Reference tab implementation with Update
- `include/ui/audioTab.h` / `src/ui/audioTab.cpp` -- Reference tab (static display)

### Rendering
- `src/rendering/animation.cpp` -- InitTextures() asset loading pattern
- `src/rendering/hud.cpp` -- DrawTextHUD() wrapper, HUD text rendering
- `src/rendering/hud.h` -- HUD function declarations

### Fonts
- `D:\My Documents\Fonts\` -- External font directory with ~170+ TTF files across 12 folders

### Consumer Files (where font changes propagate)
- `src/debug/debugmode.cpp` -- 28 DrawText calls (debug panels)
- `src/systems/effects.cpp` -- Damage popups, log messages
- `src/entities/player.cpp` -- "[E] Interact" prompt
- `src/items/item.cpp` -- Item stack amounts
- `src/ui/popup.cpp` -- Popup notifications
- `src/rendering/hud.cpp` -- HUD labels + DrawTextHUD wrapper
