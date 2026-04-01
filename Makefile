CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./include -I./lib/include
LDFLAGS = -L./lib -lraylib -lopengl32 -lgdi32 -lwinmm

SRC_DIR = src
OBJ_DIR = build
EXE = main.exe

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

app: $(OBJ)
	$(CXX) $(OBJ) -o $(EXE) $(LDFLAGS)
	cmd /c copy /Y lib\\raylib.dll .

$(OBJ_DIR):
	cmd /c mkdir $(OBJ_DIR) 2>nul

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

cln:
	cmd /c rmdir /S /Q $(OBJ_DIR) 2>nul
	cmd /c del /Q $(EXE) raylib.dll 2>nul
