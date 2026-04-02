CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./include -I./lib/include -Wno-missing-field-initializers
LDFLAGS = -L./lib -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows

SRC_DIR = src
OBJ_DIR = build
EXE = main.exe

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

app: $(OBJ)
	@cls
	$(CXX) $(OBJ) -o $(EXE) $(LDFLAGS)
	cp lib/raylib.dll .

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

cln:
	@cls
	cmd /c "del /Q $(OBJ_DIR)\*.o $(EXE) raylib.dll"
	@echo Semua file dihapus
