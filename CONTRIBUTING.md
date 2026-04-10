# Contributing

## Development Setup

### Required Tools

Install the following tools to build the project:

| Tool | Windows (scoop) | macOS (brew) | Linux (apt) |
|------|-----------------|--------------|-------------|
| **Compiler (gcc)** | `scoop install gcc` | `brew install gcc` | `apt install gcc` |
| **CMake** | `scoop install cmake` | `brew install cmake` | `apt install cmake` |
| **Ninja** | `scoop install ninja` | `brew install ninja` | `apt install ninja-build` |
| **ccache** | `scoop install ccache` | `brew install ccache` | `apt install ccache` |

### First-Time Setup

1. Install all required tools (see table above)
2. Run the setup script to download raylib:
   ```powershell
   .\setup.ps1
   ```

## Building

### Quick Start

```bash
# Configure (one time, or after adding new files)
cmake --preset ninja

# Build
cmake --build --preset ninja
```

The executable will be at `build/bin/main.exe`.

### Build Presets

| Preset | Description |
|--------|-------------|
| `ninja` | Release build with optimizations (default) |
| `ninja-debug` | Debug build with symbols |

### Manual Build (without presets)

```bash
cmake -B build -G Ninja
cmake --build build --parallel
```

### Clean Build

```bash
# PowerShell
Remove-Item -Recurse -Force build

# CMD
rmdir /s /q build

# Then rebuild
cmake --preset ninja
cmake --build --preset ninja
```

### Command Reference

| Action | PowerShell | CMD |
|--------|------------|-----|
| **Configure** | `cmake --preset ninja` | `cmake --preset ninja` |
| **Build** | `cmake --build --preset ninja` | `cmake --build --preset ninja` |
| **Clean** | `Remove-Item -Recurse -Force build` | `rmdir /s /q build` |

## Adding New Source Files

New `.cpp` files in `src/` are automatically discovered on the next CMake run. No manual changes needed.

## Troubleshooting

- **"No such file or directory" errors**: Run `.\setup.ps1` to download dependencies
- **Build errors after adding files**: Run `cmake --preset ninja` to reconfigure
