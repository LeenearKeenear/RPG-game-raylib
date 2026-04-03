# AGENTS.md

## Project Overview

This is a 2D RPG game built with C++17 and Raylib 5.5. The game runs on Windows
only and requires a Unix-like shell (Git Bash, MSYS2, or PowerShell 7) for
building. CMD is not supported.

**Key technologies:**
- C++17
- Raylib 5.5 (bundled in `lib/`)
- Make (for build orchestration)
- Clang (recommended) or MinGW-w64 GCC

---

## Build Commands

### Prerequisites
- **Clang** (recommended) or MinGW-w64 GCC
- GNU Make
- Unix-like shell (Git Bash/MSYS2/PowerShell 7)

### Commands

| Command | Shell | Purpose |
|---------|-------|---------|
| `make app` | Git Bash / MSYS2 | Build the game executable |
| `make cln` | Git Bash / MSYS2 | Clean build artifacts |
| `make -f Makefile.ps1 app` | PowerShell 7 | Build using PowerShell |
| `make -f Makefile.ps1 cln` | PowerShell 7 | Clean using PowerShell |
| `./main.exe` | Any | Run the compiled game |

### Compiler Settings
- **Standard:** C++17 (`-std=c++17`)
- **Flags:** `-Wall -Wextra -Wno-missing-field-initializers`
- **Include paths:** `-I./include -I./lib/include`

### Testing
No automated test framework is configured. Test changes manually by building
and running the game.

### Linting
No linting tools are configured. Code style is enforced manually (see below).

---

## Code Style Guidelines

### Naming Conventions

**Use PascalCase for all identifiers:**
```cpp
// Good
int GameScreenWidth;
int GameScreenHeight;
void DrawTextureText();
Vector2 PlayerPosition;

// Bad
int game_screen_width;  // snake_case not used
int gScreenWidth;       // Hungarian notation not used
```

### Magic Numbers

**AVOID magic numbers** in function parameters. Use named constants:

```cpp
// Good
int screenWidth = 1280;
int screenHeight = 720;
InitWindow(screenWidth, screenHeight, "Dungeon Game");

// Bad
InitWindow(1280, 720, "Dungeon Game");  // Magic numbers
```

### Comments

**Source code comments (.cpp, .h):** Write in Indonesian
**Documentation files (.md):** Write in English

```cpp
// Fungsi untuk generate karakter player
void DrawCharacter() {
    // Draw player sprite
    DrawTexture(playerSprite, x, y, WHITE);
}
```

**Required for:**
- Every function (brief description)
- Complex logic that isn't self-explanatory
- Non-obvious workarounds or decisions

### Formatting

- Use 4 spaces for indentation (not tabs)
- Opening brace on same line: `if (condition) {`
- One statement per line
- Vertical spacing between functions: 1 blank line

### Header Guards

Use `#pragma once` instead of traditional include guards:

```cpp
#pragma once

#include <raylib.h>
// ...
```

---

## File Organization

| File Type | Location |
|-----------|----------|
| Source files (.cpp) | `src/` |
| Header files (.h) | `include/` |
| Raylib library | `lib/` |
| Build output | `build/` (generated) |

**Include paths:** Use relative paths from project root
```cpp
#include "../include/dungeon.h"    // From src/
#include <raylib.h>                  // From lib/
```

---

## Type Conventions

### Raylib Types
Use Raylib's built-in types: `Vector2`, `Rectangle`, `Texture2D`, `Color`,
`Camera2D`, etc.

### Custom Structs
Define structs in header files with PascalCase names:

```cpp
// include/screen.h
#pragma once

typedef struct GameState {
    Texture2D Dungeon;
    Vector2 CameraPosition;
    int CurrentScreen;
} GameState;

GameState InitScreen(void);
void UpdateGame(GameState* state);
```

### Enums
Use uppercase with underscores for enum values:

```cpp
typedef enum {
    SCREEN_TITLE = 0,
    SCREEN_GAME = 1,
    SCREEN_PAUSE = 2
} GameScreen;
```

---

## Raylib Patterns

### Standard Game Loop
Follow this lifecycle pattern:

```cpp
int main(void) {
    // 1. Initialize
    GameState state = InitScreen();
    InitWindow(width, height, "Game Title");
    
    // 2. Main loop
    while (!WindowShouldClose()) {
        // 3. Update
        UpdateGame(&state);
        
        // 4. Draw to render texture (for scaling)
        DrawRenderTexture(&state);
        
        // 5. Draw UI
        DrawRenderWindows(&state);
    }
    
    // 6. Cleanup
    UnloadRenderTexture(state.Dungeon);
    CloseWindow();
    return 0;
}
```

### Resource Management
Always unload resources when done:

```cpp
UnloadTexture(playerSprite);
UnloadRenderTexture(renderTarget);
CloseWindow();
```

---

## Error Handling

### Raylib Error Checks
Check Raylib function return values when applicable:

```cpp
Texture2D texture = LoadTexture("player.png");
if (texture.id == 0) {
    // Handle loading failure
    TraceLog(LOG_ERROR, "Failed to load player.png");
}
```

### Logging
Use Raylib's TraceLog for debugging:

```cpp
TraceLog(LOG_INFO, "Game initialized");
TraceLog(LOG_WARNING, "Low memory");
TraceLog(LOG_ERROR, "Failed to load texture");
```

---

## Common Pitfalls

### Windows-Only
This project only builds on Windows. Avoid platform-specific code that assumes
POSIX compatibility without proper guards.

### Shell Requirements
The Makefile requires Unix shell features. Use Git Bash, MSYS2, or PowerShell 7
instead of CMD.

### Raylib Path
Always include Raylib from `lib/include/`, not system paths. The project
bundles a specific version.

### Render Texture Scaling
When implementing UI scaling, draw to a `RenderTexture2D` first, then draw
the texture scaled to screen. Don't assume native resolution.

---

## VSCode Configuration

The `.vscode/` folder contains basic configuration. Key files:
- `c_cpp_properties.json` - Include paths for IntelliSense
- `launch.json` - Debugging configuration (may need adjustment)

---

## Git Workflow

- **Do not commit:** `build/`, `main.exe`, `*.o` files, `AGENTS.md`
- **Do commit:** Source code, headers, Raylib library, Makefiles
- **Commit messages:** English

---

## Quick Reference

```bash
# Build the game
make app

# Run after building
./main.exe

# Clean build artifacts
make cln

# PowerShell variant
make -f Makefile.ps1 app
```
