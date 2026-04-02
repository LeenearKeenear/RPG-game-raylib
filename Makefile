CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./include -I./lib/include -Wno-missing-field-initializers
LDFLAGS = -L./lib -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows

SRC_DIR = src
OBJ_DIR = build
OBJ_DIR_CLANG = build-clang
EXE = main.exe
EXE_CLANG = main-clang.exe

SRC = $(wildcard $(SRC_DIR)/*.cpp)

app: $(OBJ_DIR)
	$(CXX) $(SRC) -o $(EXE) $(LDFLAGS) -I./include -I./lib/include
	cp lib/raylib.dll .

app-clang: $(OBJ_DIR_CLANG)
	clang++ $(SRC) -o $(EXE_CLANG) $(LDFLAGS) -I./include -I./lib/include
	cp lib/raylib.dll .

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR_CLANG):
	mkdir -p $(OBJ_DIR_CLANG)

cln:
	rm -rf $(OBJ_DIR) $(OBJ_DIR_CLANG) $(EXE) $(EXE_CLANG) raylib.dll
