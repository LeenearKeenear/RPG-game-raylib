CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./include -I./raylib/include -Wno-missing-field-initializers
LDFLAGS = -L./raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows

TMPDIR := tmp

$(TMPDIR):
	powershell -Command "if (!(Test-Path $(TMPDIR))) { New-Item -ItemType Directory -Path $(TMPDIR) -Force | Out-Null }"

export TMP := $(CURDIR)/$(TMPDIR)
export TEMP := $(TMP)

SRC_DIR = src
OBJ_DIR = build
EXE = main.exe
DLL_SOURCE = raylib/lib/raylib.dll
RAYLIB_LIB = raylib/lib/libraylib.a

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

app: $(RAYLIB_LIB) $(OBJ)
	$(CXX) $(OBJ) -o $(EXE) $(LDFLAGS)
	powershell -Command "if (Test-Path '$(DLL_SOURCE)') { Copy-Item -Force '$(DLL_SOURCE)' . }"

$(RAYLIB_LIB):
	@powershell -ExecutionPolicy Bypass -File setup.ps1

$(OBJ_DIR): $(TMPDIR)
	powershell -Command "if (!(Test-Path $(OBJ_DIR))) { New-Item -ItemType Directory $(OBJ_DIR) }"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

cln:
	powershell -Command "Stop-Process -Name main -ErrorAction SilentlyContinue; Start-Sleep -Milliseconds 500; Remove-Item -Recurse -Force $(OBJ_DIR) -ErrorAction Continue; Remove-Item -Recurse -Force $(TMPDIR) -ErrorAction Continue; Remove-Item -Force $(EXE), raylib.dll -ErrorAction SilentlyContinue; exit 0"
