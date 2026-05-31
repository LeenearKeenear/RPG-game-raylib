# Handoff: Keybinds Tab Reorganization, Font Change & Custom Keybinds

## Session Context

This handoff covers analysis and implementation planning for three tasks on the RPG-raylib project (raylib C++ dungeon game, 1280x720 virtual screen). Full codebase analysis was done in the original session -- all findings are captured here.

## Priority Order

**Step 1 first** (reorganize keybinds display), then **Step 2** (change font), then **Step 3** (customizable keybinds). Steps 1-2 are independent of Step 3 but doing them first gives a clean foundation. Step 3 can be built incrementally on top of Step 1's reorganized display.

**IMPORTANT:** Step 3 is the largest and most complex change. The agent working on it needs to understand the full input pipeline (input.cpp -> movement/combat/interaction/inventory/hud).

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

## Step 3: Customizable Keybinds

### What This Feature Means

The player can click a keybind entry in the options screen, press a new key, and that action gets remapped. The binding persists across game restarts via a JSON config file.

### Current Architecture Problem

Right now, every key is **hardcoded** in three separate places:

| Location | What's Hardcoded | Complexity |
|----------|-----------------|------------|
| `src/systems/input.cpp:20-49` | All gameplay keybind checks (WASD, E, I, M, 1-4, R, K, B, Q, Ctrl) | ~15 entries, some dual-key (W+UP) |
| `src/core/main.cpp:116` | Pause key (`KEY_GRAVE`) | 1 entry, handled outside input system |
| `src/debug/debugmode.cpp:296-305` | Debug keys (`KEY_TAB`, `KEY_BACKSLASH`, `KEY_RIGHT_BRACKET`) | 3 entries, handled outside input system |

The display (`keybindsTab.cpp`) is a separate hardcoded array with zero connection to the actual bindings.

### Architecture for Custom Keybinds

#### A. New Data Structures (in `include/systems/input.h` or new `include/systems/keybinds.h`)

```cpp
// Enum of every BINDABLE action (NOT the same as InputState fields)
enum BindableAction {
    // Movement (each gets ONE bindable key; Arrow keys stay hardcoded as alternates)
    BIND_MOVE_UP,
    BIND_MOVE_DOWN,
    BIND_MOVE_LEFT,
    BIND_MOVE_RIGHT,
    
    // Actions
    BIND_INTERACT,       // currently KEY_E
    BIND_INVENTORY,      // currently KEY_I
    BIND_MAP,            // currently KEY_M
    BIND_DROP_ITEM,      // currently KEY_Q
    BIND_DROP_ALL,       // currently KEY_LEFT_CONTROL
    BIND_REVIVE,         // currently KEY_R
    BIND_PREV_MAP,       // currently KEY_B
    BIND_ATTACK,         // currently MOUSE_BUTTON_LEFT
    BIND_DASH,           // currently MOUSE_BUTTON_RIGHT
    
    // Hotbar
    BIND_SLOT_1,         // currently KEY_ONE
    BIND_SLOT_2,         // currently KEY_TWO
    BIND_SLOT_3,         // currently KEY_THREE
    BIND_SLOT_4,         // currently KEY_FOUR
    
    // System (these live in main.cpp/debugmode.cpp -- harder to move)
    BIND_PAUSE,          // currently KEY_GRAVE
    BIND_DEBUG_TOGGLE,   // currently KEY_TAB
    
    BIND_COUNT
};

// A single key binding
struct KeyBinding {
    int key;             // Raylib KeyboardKey or MouseButton constant
    // Note: Use -1 for "unbound". Mouse = MOUSE_BUTTON_LEFT (0) etc.
};

// The full binding map
struct KeyBindings {
    KeyBinding bindings[BIND_COUNT];
    
    // Load defaults matching current hardcoded keys
    void SetDefaults();
    
    // Get/set individual bindings
    int GetKey(BindableAction action) const;
    void SetKey(BindableAction action, int newKey);
};
```

**Design decision: Dual-key for movement.** WASD + Arrow keys both work for movement currently. The cleanest approach for v1:
- Each movement action has ONE **bindable** key (default: W, S, A, D)
- The Arrow keys stay **hardcoded as alternates** in PollInput()
- The display shows both: "W / Arrow Up" for Move Up
- If user rebinds W to E, movement still responds to Arrow Up

#### B. Default Bindings

```cpp
void KeyBindings::SetDefaults() {
    bindings[BIND_MOVE_UP]    = {KEY_W};
    bindings[BIND_MOVE_DOWN]  = {KEY_S};
    bindings[BIND_MOVE_LEFT]  = {KEY_A};
    bindings[BIND_MOVE_RIGHT] = {KEY_D};
    bindings[BIND_INTERACT]   = {KEY_E};
    bindings[BIND_INVENTORY]  = {KEY_I};
    bindings[BIND_MAP]        = {KEY_M};
    bindings[BIND_DROP_ITEM]  = {KEY_Q};
    bindings[BIND_DROP_ALL]   = {KEY_LEFT_CONTROL};
    bindings[BIND_REVIVE]     = {KEY_R};
    bindings[BIND_PREV_MAP]   = {KEY_B};
    bindings[BIND_ATTACK]     = {MOUSE_BUTTON_LEFT};
    bindings[BIND_DASH]       = {MOUSE_BUTTON_RIGHT};
    bindings[BIND_SLOT_1]     = {KEY_ONE};
    bindings[BIND_SLOT_2]     = {KEY_TWO};
    bindings[BIND_SLOT_3]     = {KEY_THREE};
    bindings[BIND_SLOT_4]     = {KEY_FOUR};
    bindings[BIND_PAUSE]      = {KEY_GRAVE};
    bindings[BIND_DEBUG_TOGGLE] = {KEY_TAB};
}
```

#### C. Modified PollInput() (in `input.cpp`)

Currently:
```cpp
Current.moveUp = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
Current.interact = IsKeyPressed(KEY_E);
```

After:
```cpp
// Globally accessible bindings
extern KeyBindings gKeybindings;

void PlayerInput::PollInput(void) {
    // Movement: bindable key + hardcoded Arrow alternates
    Current.moveUp    = IsKeyDown(KEY_UP) || IsKeyDown(gKeybindings.GetKey(BIND_MOVE_UP));
    Current.moveDown  = IsKeyDown(KEY_DOWN) || IsKeyDown(gKeybindings.GetKey(BIND_MOVE_DOWN));
    Current.moveLeft  = IsKeyDown(KEY_LEFT) || IsKeyDown(gKeybindings.GetKey(BIND_MOVE_LEFT));
    Current.moveRight = IsKeyDown(KEY_RIGHT) || IsKeyDown(gKeybindings.GetKey(BIND_MOVE_RIGHT));
    
    // Actions: single key lookup
    Current.interact      = IsKeyPressed(gKeybindings.GetKey(BIND_INTERACT));
    Current.revive        = IsKeyPressed(gKeybindings.GetKey(BIND_REVIVE));
    Current.toggleInventory = IsKeyPressed(gKeybindings.GetKey(BIND_INVENTORY));
    Current.toggleMap     = IsKeyPressed(gKeybindings.GetKey(BIND_MAP));
    Current.dropItem      = IsKeyPressed(gKeybindings.GetKey(BIND_DROP_ITEM));
    Current.dropItemAll   = IsKeyDown(gKeybindings.GetKey(BIND_DROP_ALL));
    Current.goBack        = IsKeyPressed(gKeybindings.GetKey(BIND_PREV_MAP));
    Current.testLoseHP    = IsKeyPressed(KEY_K); // keep dead code as-is for now
    
    // Slots
    Current.selectSlot1 = IsKeyPressed(gKeybindings.GetKey(BIND_SLOT_1));
    Current.selectSlot2 = IsKeyPressed(gKeybindings.GetKey(BIND_SLOT_2));
    Current.selectSlot3 = IsKeyPressed(gKeybindings.GetKey(BIND_SLOT_3));
    Current.selectSlot4 = IsKeyPressed(gKeybindings.GetKey(BIND_SLOT_4));
    
    // Mouse (keep direct -- these are hardware-specific)
    Current.leftClickPressed  = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    Current.rightClickPressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    // ... rest of mouse and ctrl unchanged
    
    // Ctrl modifier (special case -- used as modifier key, not an action)
    Current.ctrlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    // Note: BIND_DROP_ALL reuses Ctrl -- if user rebinds drop-all to something else,
    // ctrlDown modifier behavior stays unchanged
}
```

**Important:** See note in the Ctrl section above. `ctrlDown` is used as a general modifier (e.g., for inventory stack splitting in `hud.cpp`), NOT just for drop-all. The `KEY_LEFT_CONTROL`/`KEY_RIGHT_CONTROL` check should remain hardcoded. Only `dropItemAll` uses the binding.

#### D. main.cpp and debugmode.cpp Changes

For `BIND_PAUSE` and `BIND_DEBUG_TOGGLE`, these are currently checked directly in `main.cpp:116` and `debugmode.cpp:296`:

```cpp
// main.cpp - change from:
if (IsKeyPressed(KEY_GRAVE))
// to:
if (IsKeyPressed(gKeybindings.GetKey(BIND_PAUSE)))

// debugmode.cpp - change from:
if (IsKeyPressed(KEY_TAB))
// to:
if (IsKeyPressed(gKeybindings.GetKey(BIND_DEBUG_TOGGLE)))
```

This requires including the keybinds header in `main.cpp` and `debugmode.cpp`.

### Key Capture UI

#### New files to create

| File | Purpose |
|------|---------|
| `include/ui/keybindsTab.h` | **Modify** -- add `UpdateKeybindsTab()` declaration |
| `src/ui/keybindsTab.cpp` | **Rewrite** -- add rebinding UI logic |
| `include/systems/keybinds.h` | **New** -- KeyBinding struct, BindableAction enum, KeyBindings class |
| `src/systems/keybinds.cpp` | **New** -- default bindings + JSON serialization |
| (optional) `include/systems/keybinds_internals.h` | **New** -- if you want to separate public API from internal |

#### UI Flow

1. **Normal state**: Display keybinds in two columns (Main/Debugging) from Step 1
2. **Waiting for input state**: When user clicks a keybind row:
   - Highlight that row (e.g., RED background or blinking)
   - Show "Press a key..." hint text
   - Call `GetKeyPressed()` in a loop until a key is pressed
   - Update the binding for that action
   - Exit waiting state
3. **Conflict detection** (optional for v1): Check if the new key is already bound to another action. If so, either swap them or warn the player.

#### UpdateKeybindsTab() Design

Follow the `UpdateVideoTab()` pattern from `videoTab.cpp`:

```cpp
// New function declaration in keybindsTab.h:
bool UpdateKeybindsTab(
    Vector2 mousePosition,
    bool mouseClicked,
    int startX,
    int startY
);
```

Returns `true` if bindings changed (to trigger redraw or save).

Inside, track state:
```cpp
static BindableAction waitingForAction = BIND_COUNT; // BIND_COUNT = not waiting
static int frameCounter = 0;

// If waiting for a key press
if (waitingForAction != BIND_COUNT) {
    int newKey = GetKeyPressed();  // raylib: returns 0 if no key pressed this frame
    if (newKey != 0) {
        gKeybindings.SetKey(waitingForAction, newKey);
        waitingForAction = BIND_COUNT;
        return true; // binding changed
    }
    // Show "Press a key..." animation
    frameCounter++;
    return false;
}

// Check clicks on keybind rows
// ... (hit-test each row's rectangle, convert to BindableAction, set waitingForAction)
```

**Key raylib API notes:**
- `GetKeyPressed()` returns the next queued key press (0 if none). It only returns ONE per call.
- Poll it repeatedly or use `GetCharPressed()` if you want character input.
- For mouse binding capture, use `GetMousePressed()` which returns the mouse button.
- For waiting, loop `GetKeyPressed()` until non-zero, using a frame count timeout.

Actually, `GetKeyPressed()` returns keys that were pressed since the last call. The simplest approach:
1. Call `GetKeyPressed()` repeatedly until it returns 0 (drain the queue)
2. The first non-zero result is the new binding
3. If queue is empty, keep waiting

But this has a problem: if the mouse button is used to CLICK the binding, the click itself registers as a mouse button press. Solution: use a "debounce" frame -- when entering capture mode, discard input for 1-2 frames, then start capturing.

### Wiring into OptionsScreen

In `include/ui/pauseMenu.h` and `src/ui/pauseMenu.cpp`:

1. **OptionsScreen::Update()**: Add case for `selectedTab == 2`:
```cpp
if (selectedTab == 2) {
    if (UpdateKeybindsTab(mousePosition, mouseClicked, startX, startY)) {
        // Bindings changed -- optionally mark dirty for auto-save
    }
}
```

2. **OptionsScreen::Draw()**: Already calls `DrawKeybindsTab()` for case 2. Keep that.

### Config File Persistence

**Format:** JSON using `nlohmann/json` (already used by the project at `lib/json/include/nlohmann/json.hpp`)

**File path:** `saves/keybinds.json` (follows existing save file pattern)

**Save on:**
- Every rebind (immediate save for crash safety)
- Or save on exiting options screen

**Recommendation:** Save immediately on each rebind. Use the atomic write pattern from `WriteSaveFile()` in `game_state_saver.cpp` (write to `.tmp`, then rename).

#### Save Function

```cpp
void SaveKeybindings(const KeyBindings& kb) {
    json j;
    for (int i = 0; i < BIND_COUNT; i++) {
        j[std::to_string(i)] = kb.bindings[i].key;
    }
    
    std::string path = "saves/keybinds.json";
    std::string tmpPath = path + ".tmp";
    
    std::ofstream file(tmpPath);
    file << j.dump(4);
    file.close();
    
    std::filesystem::rename(tmpPath, path);
}
```

#### Load Function

```cpp
KeyBindings LoadKeybindings() {
    KeyBindings kb;
    kb.SetDefaults();
    
    std::string path = "saves/keybinds.json";
    if (!std::filesystem::exists(path))
        return kb; // return defaults if no file
    
    try {
        std::ifstream file(path);
        json j;
        file >> j;
        
        for (int i = 0; i < BIND_COUNT; i++) {
            std::string key = std::to_string(i);
            if (j.contains(key) && j[key].is_number_integer()) {
                kb.bindings[i].key = j[key].get<int>();
            }
        }
    } catch (...) {
        // On error, just use defaults
        kb.SetDefaults();
    }
    return kb;
}
```

#### Where to Load

In `main()` or `InitScreen()`:
```cpp
// After window init, before game loop:
gKeybindings = LoadKeybindings();
```

Or lazily in the first PollInput() call (simpler, avoids init-order issues).

### Display Integration

The `DrawKeybindsTab()` needs to show the CURRENT binding from `gKeybindings`, not the defaults. The key-to-name conversion requires a helper function:

```cpp
const char* GetKeyDisplayName(int key) {
    switch (key) {
        case KEY_W: return "W";
        case KEY_UP: return "Arrow Up";
        case KEY_SPACE: return "Space";
        case KEY_LEFT_CONTROL: return "Left Ctrl";
        case MOUSE_BUTTON_LEFT: return "Mouse Left";
        case MOUSE_BUTTON_RIGHT: return "Mouse Right";
        // ... all relevant keys
        default: return TextFormat("Key %d", key); // fallback for unknown
    }
}
```

Raylib defines all key constants in `raylib.h` (`KEY_A` through `KEY_Z`, `KEY_F1` through `KEY_F12`, etc.).

### Full File Change Summary for Step 3

| File | Action | Change |
|------|--------|--------|
| `include/systems/keybinds.h` | **CREATE** | `BindableAction` enum, `KeyBinding` struct, `KeyBindings` class, `extern KeyBindings gKeybindings`, `GetKeyDisplayName()` |
| `src/systems/keybinds.cpp` | **CREATE** | `SetDefaults()`, `SaveKeybindings()`, `LoadKeybindings()`, `GetKeyDisplayName()` implementation |
| `include/ui/keybindsTab.h` | **MODIFY** | Add `UpdateKeybindsTab()` declaration |
| `src/ui/keybindsTab.cpp` | **REWRITE** | Use `gKeybindings` data for display, add label display for key names, add capture mode state machine |
| `src/systems/input.cpp` | **MODIFY** | Include `keybinds.h`, use `gKeybindings.GetKey()` in PollInput() |
| `include/systems/input.h` | **NO CHANGE** | InputState fields stay the same |
| `src/core/main.cpp` | **MODIFY** | Include `keybinds.h`, use `gKeybindings.GetKey(BIND_PAUSE)` |
| `src/debug/debugmode.cpp` | **MODIFY** | Include `keybinds.h`, use `gKeybindings.GetKey(BIND_DEBUG_TOGGLE)` |
| `src/ui/pauseMenu.cpp` | **MODIFY** | Add `selectedTab == 2` update case in OptionsScreen::Update() |
| `src/core/screen_handler.cpp` | **MODIFY** | Or `main.cpp` -- call `LoadKeybindings()` during init |
| `CMakeLists.txt` | **MODIFY** | Add `src/systems/keybinds.cpp` to the unity build list |

### Build Order Recommendation

1. Create `include/systems/keybinds.h` and `src/systems/keybinds.cpp` (data structures + defaults + save/load)
2. Verify build with defaults only (no gameplay changes yet)
3. Update `src/systems/input.cpp` to use `gKeybindings.GetKey()` (replaces hardcoded constants)
4. Update `main.cpp` pause key and `debugmode.cpp` debug toggle
5. Add `UpdateKeybindsTab()` to keybindsTab with capture UI
6. Wire into `OptionsScreen::Update()`
7. Integrate the keybind display in `DrawKeybindsTab()` to read from `gKeybindings`
8. Final test: rebind a key, confirm it persists across restart

### Common Pitfalls

- **GetKeyPressed() only fires once per frame** -- you must poll it repeatedly in a loop to drain the queue
- **Mouse button vs keyboard key** -- `GetKeyPressed()` returns keyboard keys only. Use `GetMousePressed()` or check `IsMouseButtonPressed()` separately in capture mode
- **Ctrl modifier split** -- `ctrlDown` in InputState is used as a modifier (e.g., for inventory stack splitting), NOT just for `dropItemAll`. Keep it hardcoded; only make `dropItemAll` go through the binding system
- **Dead code K key** -- `testLoseHP` has no consumers. Either remove it from the bindable list or mark it clearly
- **Pause key in main.cpp** is checked BEFORE `PollInput()` -- the pause toggle happens outside the input system. To make it rebindable, you need the binding lookup in `main.cpp` too

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
