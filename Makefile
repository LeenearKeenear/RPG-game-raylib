CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./lib/include
LDFLAGS = -L./lib/lib -lraylib -lopengl32 -lgdi32 -lwinmm

SRC_DIR = src
OBJ_DIR = build
EXE = main.exe

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_GCC = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/g++/%.o)
OBJ_CLANG = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/clang++/%.o)

app: CC = g++
app: CXX = g++
app: $(OBJ_DIR)/g++ $(OBJ_GCC)
	@rm -f $(EXE)
	$(CXX) $(OBJ_GCC) -o $(EXE) $(LDFLAGS)
	cp lib/lib/raylib.dll .
	@echo Build selesai!

app-clang: CC = clang++
app-clang: CXX = clang++
app-clang: $(OBJ_DIR)/clang++ $(OBJ_CLANG)
	@rm -f $(EXE)
	$(CXX) $(OBJ_CLANG) -o $(EXE) $(LDFLAGS)
	cp lib/lib/raylib.dll .
	@echo Build selesai!

$(OBJ_DIR)/g++:
	@mkdir -p $(OBJ_DIR)/g++

$(OBJ_DIR)/clang++:
	@mkdir -p $(OBJ_DIR)/clang++

$(OBJ_DIR)/g++/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/clang++/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

cln:
	rm -rf $(OBJ_DIR) $(EXE) raylib.dll
	@echo Build dibersihkan!
