CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./lib/include
LDFLAGS = -L./lib/lib -lraylib -lopengl32 -lgdi32 -lwinmm

SRC_DIR = src
OBJ_DIR = build
EXE = main.exe

app: CC = g++
app: CXX = g++
app:
	@rm -f $(EXE)
	@mkdir -p $(OBJ_DIR)/g++
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/logic.cpp -o $(OBJ_DIR)/g++/logic.o
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/main.cpp -o $(OBJ_DIR)/g++/main.o
	$(CXX) $(OBJ_DIR)/g++/logic.o $(OBJ_DIR)/g++/main.o -o $(EXE) $(LDFLAGS)
	cp lib/lib/raylib.dll .
	@echo Build selesai!

app-clang: CC = clang++
app-clang: CXX = clang++
app-clang:
	@rm -f $(EXE)
	@mkdir -p $(OBJ_DIR)/clang++
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/logic.cpp -o $(OBJ_DIR)/clang++/logic.o
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/main.cpp -o $(OBJ_DIR)/clang++/main.o
	$(CXX) $(OBJ_DIR)/clang++/logic.o $(OBJ_DIR)/clang++/main.o -o $(EXE) $(LDFLAGS)
	cp lib/lib/raylib.dll .
	@echo Build selesai!

cln:
	rm -rf $(OBJ_DIR) $(EXE) raylib.dll
	@echo Build dibersihkan!
