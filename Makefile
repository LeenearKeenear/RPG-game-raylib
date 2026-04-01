CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./lib/include
LDFLAGS = -L./lib/lib -lraylib -lopengl32 -lgdi32 -lwinmm

OBJ_DIR = build
SRC_DIR = src

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

EXE = main.exe

app: $(OBJ)
	rm -f $(EXE)
	$(CXX) $(OBJ) -o $(EXE) $(LDFLAGS)
	cp lib/lib/raylib.dll .
	@echo Build selesai!

app-clang: CXX = clang++
app-clang: app

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

cln:
	rm -rf $(OBJ_DIR) $(EXE) raylib.dll
	@echo Build dibersihkan!
