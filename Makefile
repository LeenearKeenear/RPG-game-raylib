CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./include -I./lib/include
LDFLAGS = -L./lib -lraylib -lopengl32 -lgdi32 -lwinmm

SRC_DIR = src
OBJ_DIR = build
EXE = main.exe

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

app: $(OBJ)
	@rm -f $(EXE)
	$(CXX) $(OBJ) -o $(EXE) $(LDFLAGS)
	cp lib/raylib.dll .
	@echo Build selesai!

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

cln:
	rm -rf $(OBJ_DIR) $(EXE) raylib.dll
	@echo Build dibersihkan!
