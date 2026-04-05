#include "../include/screen.h"
#include "../raylib/include/raylib.h"
#include "../raylib/include/raymath.h"

// ======================
// CONSTANTS
// ======================
const float SCALE_MONITOR = 0.7f;
const float SCALE_MIN_MONITOR = 0.4f;

// ======================
// HELPER FUNCTIONS
// ======================
static float CalculateScale(int WindowWidth, int WindowHeight)
{
    float ScaleX = (float)WindowWidth / Config::GAME_WIDTH;
    float ScaleY = (float)WindowHeight / Config::GAME_HEIGHT;
    return (ScaleX < ScaleY) ? ScaleX : ScaleY;
}

static Vector2 GetVirtualMouse(GameState *Game)
{
    Vector2 Mouse = GetMousePosition();

    float OffsetX = (Game->WindowWidth - (Config::GAME_WIDTH * Game->ScaleMultiplier)) * 0.5f;
    float OffsetY = (Game->WindowHeight - (Config::GAME_HEIGHT * Game->ScaleMultiplier)) * 0.5f;

    Vector2 VirtualMouse;
    VirtualMouse.x = (Mouse.x - OffsetX) / Game->ScaleMultiplier;
    VirtualMouse.y = (Mouse.y - OffsetY) / Game->ScaleMultiplier;

    VirtualMouse = Vector2Clamp(
        VirtualMouse,
        (Vector2){0, 0},
        (Vector2){(float)Config::GAME_WIDTH, (float)Config::GAME_HEIGHT});

    return VirtualMouse;
}

// ======================
// INIT
// ======================
GameState InitScreen()
{
    GameState Game = {0};

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(Config::GAME_WIDTH, Config::GAME_HEIGHT, "Dungeon Game");

    Game.WindowWidth = (int)(GetMonitorWidth(0) * SCALE_MONITOR);
    Game.WindowHeight = (int)(GetMonitorHeight(0) * SCALE_MONITOR);
    Game.ScaleMultiplier = CalculateScale(Game.WindowWidth, Game.WindowHeight);

    SetWindowSize(Game.WindowWidth, Game.WindowHeight);

    SetWindowMinSize(
        (int)(GetMonitorWidth(0) * SCALE_MIN_MONITOR),
        (int)(GetMonitorHeight(0) * SCALE_MIN_MONITOR));

    Game.Dungeon = LoadRenderTexture(Config::GAME_WIDTH, Config::GAME_HEIGHT);
    SetTextureFilter(Game.Dungeon.texture, TEXTURE_FILTER_BILINEAR);

    SetTargetFPS(60);

    return Game;
}

// ======================
// UPDATE
// ======================
void UpdateGameState(GameState *Game)
{
    Game->WindowWidth = GetScreenWidth();
    Game->WindowHeight = GetScreenHeight();
    Game->ScaleMultiplier = CalculateScale(Game->WindowWidth, Game->WindowHeight);
}

// ======================
// DRAW TO VIRTUAL SCREEN
// ======================
void DrawRenderTexture(GameState *Game)
{
    Vector2 Mouse = GetMousePosition();
    Vector2 VirtualMouse = GetVirtualMouse(Game);

    BeginTextureMode(Game->Dungeon);
    ClearBackground(BLACK);

    DrawRectangle(10, 10, 300, 80, WHITE);
    DrawText(
        TextFormat("Mouse: [%i, %i]", (int)Mouse.x, (int)Mouse.y),
        20, 20, 20, BLACK);
    DrawText(
        TextFormat("Virtual Mouse: [%i, %i]", (int)VirtualMouse.x, (int)VirtualMouse.y),
        20, 50, 20, BLACK);

    EndTextureMode();
}

// ======================
// DRAW TO REAL SCREEN
// ======================
void DrawRenderWindow(GameState *Game)
{
    float OffsetX = (Game->WindowWidth - (Config::GAME_WIDTH * Game->ScaleMultiplier)) * 0.5f;
    float OffsetY = (Game->WindowHeight - (Config::GAME_HEIGHT * Game->ScaleMultiplier)) * 0.5f;

    Rectangle Source = {
        0, 0,
        (float)Config::GAME_WIDTH,
        -(float)Config::GAME_HEIGHT};

    Rectangle Dest = {
        OffsetX, OffsetY,
        Config::GAME_WIDTH * Game->ScaleMultiplier,
        Config::GAME_HEIGHT * Game->ScaleMultiplier};

    BeginDrawing();
    ClearBackground(BLACK);

    DrawTexturePro(
        Game->Dungeon.texture,
        Source,
        Dest,
        (Vector2){0, 0},
        0.0f,
        WHITE);

    EndDrawing();
}