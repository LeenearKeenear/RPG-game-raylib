CXX = g++
CXXFLAGS = -Wall -Wextra -IC:/raylib-5.5_win32_mingw-w64/include
LDFLAGS = -LC:/raylib-5.5_win32_mingw-w64/lib -lraylib -lopengl32 -lgdi32 -lwinmm

OBJ_DIR = build

SRC = main.cpp logic.cpp screen_handler.cpp
OBJ = $(SRC:%.cpp=$(OBJ_DIR)/%.o)

EXE = main.exe

app: $(OBJ)
	@del /Q $(EXE)
	@$(CXX) $(OBJ) -o $(EXE) $(LDFLAGS)
	@echo Build selesai

$(OBJ_DIR)/%.o: %.cpp
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

cln:
	@cls
	@del /Q $(OBJ_DIR)\*.o $(EXE)