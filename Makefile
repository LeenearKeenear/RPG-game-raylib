CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./include -I./lib/include -Wno-missing-field-initializers
LDFLAGS = -L./lib -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows

SRC_DIR = src
OBJ_DIR = build
EXE = main.exe

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

app: $(OBJ)
	$(CXX) $(OBJ) -o $(EXE) $(LDFLAGS)
	powershell -Command "Copy-Item -Force lib/raylib.dll ."

$(OBJ_DIR):
	powershell -Command "if (!(Test-Path build)) { New-Item -ItemType Directory build }"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

cln:
	powershell -Command "Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue; exit 0"
	powershell -Command "Remove-Item -Force main.exe, raylib.dll -ErrorAction SilentlyContinue; exit 0"
