CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./include -I./lib/raylib/include -Wno-missing-field-initializers
LDFLAGS = -L./lib/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows

TMPDIR := tmp

$(TMPDIR):
	mkdir -p $(TMPDIR)

export TMP := $(CURDIR)/$(TMPDIR)
export TEMP := $(TMP)

SRC_DIR = src
OBJ_DIR = build
EXE = main.exe
DLL_SOURCE = lib/raylib/lib/raylib.dll

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

app: setup $(OBJ)
	$(CXX) $(OBJ) -o $(EXE) $(LDFLAGS)
	powershell -Command "if (Test-Path '$(DLL_SOURCE)') { Copy-Item -Force '$(DLL_SOURCE)' . }"

setup:
	@powershell -ExecutionPolicy Bypass -File setup.ps1

$(OBJ_DIR): $(TMPDIR)
	powershell -Command "if (!(Test-Path $(OBJ_DIR))) { New-Item -ItemType Directory $(OBJ_DIR) }"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

cln:
	powershell -Command "Remove-Item -Recurse -Force $(OBJ_DIR) -ErrorAction Continue; Remove-Item -Recurse -Force $(TMPDIR) -ErrorAction Continue; Remove-Item -Force $(EXE), raylib.dll -ErrorAction Continue; exit 0"