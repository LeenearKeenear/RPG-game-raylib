# ============================================================================
#                        ⚠️  DEPRECATED MAKEFILE  ⚠️
# ============================================================================
# This Makefile is no longer maintained!
#
# PLEASE USE CMake + Ninja instead:
#   cmake --preset ninja
#   cmake --build --preset ninja
#
# See CONTRIBUTING.md for detailed instructions.
# ============================================================================

CXX = g++
CXXFLAGS = -Wall -Wextra -Wno-missing-field-initializers -std=c++17 -I./lib/raylib/include -I./lib/tileson -I./include
LDFLAGS = -L./lib/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -lstdc++fs
DLL_SOURCE = ./lib/raylib/lib/raylib.dll

ifeq ($(OS),Windows_NT)
	CORES := $(shell powershell -Command "(Get-CimInstance Win32_Processor).NumberOfLogicalProcessors")
else
	CORES := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
endif

MAKEFLAGS = -j$(CORES)

# ============================================================================
# DEPRECATION WARNING - Prints on every make command
# ============================================================================
DEPRECATION_WARNING = echo ""; echo "=============================================================="; echo "  ⚠️  WARNING: This Makefile is deprecated!"; echo "  Please use CMake instead. See CONTRIBUTING.md"; echo "=============================================================="; echo ""

.PHONY: cores help
cores:
	@$(DEPRECATION_WARNING)
	@echo Detected cores: $(CORES)

help:
	@$(DEPRECATION_WARNING)
	@echo "=============================================="
	@echo "  Build Options (DEPRECATED - Use CMake!)"
	@echo "=============================================="
	@echo "  make app        - Build (parallel compilation)"
	@echo "  make refresh    - Clean and rebuild"
	@echo "  make cln        - Clean build artifacts"
	@echo "  make cores      - Show detected CPU cores"
	@echo ""

TMPDIR := tmp

$(TMPDIR):
	@powershell -Command "if (!(Test-Path $(TMPDIR))) { New-Item -ItemType Directory -Path $(TMPDIR) }"

export TMP := $(CURDIR)/$(TMPDIR)
export TEMP := $(TMP)

SRC_DIR = src
OBJ_DIR = build
EXE = main.exe

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

app: setup $(OBJ)
	@$(DEPRECATION_WARNING)
	@echo ""
	@echo "=============================================="
	@echo "  Building with $(CORES) parallel jobs..."
	@echo "=============================================="
	@echo ""
	$(CXX) $(OBJ) -o $(EXE) $(LDFLAGS)
	@echo ""
	@powershell -Command "if (Test-Path '$(DLL_SOURCE)') { Write-Host '[DLL] Copying raylib.dll...' -ForegroundColor Cyan; Copy-Item -Force '$(DLL_SOURCE)' . }"

setup:
	@$(DEPRECATION_WARNING)
	@powershell -ExecutionPolicy Bypass -File setup.ps1

$(OBJ_DIR): $(TMPDIR)
	@powershell -Command "if (!(Test-Path $(OBJ_DIR))) { New-Item -ItemType Directory $(OBJ_DIR) }"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

cln:
	@$(DEPRECATION_WARNING)
	@echo ""
	@echo "[CLEAN] Removing build artifacts..."
	@powershell -Command "Remove-Item -Recurse -Force $(OBJ_DIR) -ErrorAction Continue; Remove-Item -Recurse -Force $(TMPDIR) -ErrorAction Continue; Remove-Item -Force $(EXE), raylib.dll -ErrorAction Continue; exit 0"
	@echo "[CLEAN] Done."
	@echo ""

refresh:
	@$(DEPRECATION_WARNING)
	@make cln
	@make app