#pragma once
#include <raylib.h>
#include "map.h"
#include "tileset.h"
#include "screen.h"

class Player
{
public:
    void Init(Map *Map);
    void Update();
    void Render();

private:
    Vector2 Position;
    Vector2 Velocity;
    int TileSize = 32;
    float Speed = 4.0f;
    Tileset *TilesetRef;
    Map *MapRef;
    TileCharType TileCharacter;
    bool CanMove(TilePos NewTilePos);
};