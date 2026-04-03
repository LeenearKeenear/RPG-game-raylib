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
	@cls
	$(CXX) $(SRC) -o $(EXE) $(LDFLAGS) -I./include -I./raylib/include
	@echo file app telah dibuat

app-clang: $(OBJ_DIR_CLANG)
	@cls
	clang++ $(SRC) -o $(EXE_CLANG) $(LDFLAGS) -I./include -I./raylib/include
	@echo file app telah dibuat

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR_CLANG):
	mkdir -p $(OBJ_DIR_CLANG)

cln:
	@cls
	@del /Q $(OBJ_DIR) $(OBJ_DIR_CLANG) $(EXE) $(EXE_CLANG) raylib.dll
	@echo Semua file dihapus